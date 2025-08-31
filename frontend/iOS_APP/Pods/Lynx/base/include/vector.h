// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef BASE_INCLUDE_VECTOR_H_
#define BASE_INCLUDE_VECTOR_H_

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <limits>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>

#ifdef DEBUG
#define BASE_VECTOR_DCHECK(...) assert(__VA_ARGS__)
#else
#define BASE_VECTOR_DCHECK(...)
#endif

/**
 * Inline annotations to precisely control functions for benefits of binary
 * size.
 */
#if _MSC_VER
#define BASE_VECTOR_INLINE inline __forceinline
#define BASE_VECTOR_NEVER_INLINE __declspec(noinline)
#else
#define BASE_VECTOR_INLINE inline __attribute__((always_inline))
#define BASE_VECTOR_NEVER_INLINE __attribute__((noinline))
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-template"

namespace lynx {
namespace base {

namespace {  // NOLINT

BASE_VECTOR_INLINE void* HeapAlloc(const size_t size) {
  return std::malloc(size);
}

BASE_VECTOR_INLINE void HeapFree(void* ptr) { std::free(ptr); }

/**
 * Checks if a type is template instance of another type.
 * For example:
 *     is_instance<std::shared_ptr<std::string>, std::shared_ptr> = true
 */
template <class, template <class, class...> class>
struct is_instance : public std::false_type {};

template <class... Ts, template <class, class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type {};

/**
 * By default, std::is_trivially_copyable_v<std::pair<int, int>> is false which
 * causes Vector<std::pair<int, int>> to be treated with non-trivial
 * element type. We check first and second elements of std::pair<> and treat the
 * whole type as trivial if both elements are trivial.
 *
 * From C++23, the standard was updated to mandate that if `T1` and `T2` are
 * both trivially copyable, then `std::pair<T1, T2>` must also be trivially
 * copyable.
 */
template <typename T, bool is_pair>
struct IsTrivial {};

template <typename T>
struct IsTrivial<T, false> {
  static constexpr auto value =
      std::is_trivially_destructible_v<T> && std::is_trivially_copyable_v<T>;
};

template <typename T>
struct IsTrivial<T, true> {
  static constexpr auto value =
      std::is_trivially_destructible_v<T> &&
      IsTrivial<typename T::first_type,
                is_instance<typename T::first_type, std::pair>{}>::value &&
      IsTrivial<typename T::second_type,
                is_instance<typename T::second_type, std::pair>{}>::value;
};

// Check if T has declared flag `TriviallyDestructibleAfterMoveInBaseVector`.
// When this optimization hint is turned on, if one T instance is moved away,
// its destructor will not be called.
template <typename T, typename = void>
struct has_trivially_destructible_after_move_flag : std::false_type {};

template <typename T>
struct has_trivially_destructible_after_move_flag<
    T, std::void_t<typename T::TriviallyDestructibleAfterMoveInBaseVector>>
    : std::true_type {};

template <typename T, bool is_pair>
struct IsTriviallyDestructibleAfterMove {};

template <typename T>
struct IsTriviallyDestructibleAfterMove<T, false> {
  static constexpr auto value =
      std::is_trivially_destructible_v<T> ||
      has_trivially_destructible_after_move_flag<T>::value;
};

template <typename T>
struct IsTriviallyDestructibleAfterMove<T, true> {
  static constexpr auto value =
      (std::is_trivially_destructible_v<typename T::first_type> ||
       IsTriviallyDestructibleAfterMove<
           typename T::first_type,
           is_instance<typename T::first_type, std::pair>{}>::value) &&
      (std::is_trivially_destructible_v<typename T::second_type> ||
       IsTriviallyDestructibleAfterMove<
           typename T::second_type,
           is_instance<typename T::second_type, std::pair>{}>::value);
};

// Check if T has declared flag `TriviallyRelocatableInBaseVector`.
// When this optimization hint is turned on, the instances of type T can be
// moved as a whole(if multiple instances on linear memory) to another piece of
// memory using the memcpy or memmove method, and the T type instance in the
// original memory does not need destruction.
template <typename T, typename = void>
struct has_trivially_relocatable_flag : std::false_type {};

template <typename T>
struct has_trivially_relocatable_flag<
    T, std::void_t<typename T::TriviallyRelocatableInBaseVector>>
    : std::true_type {};

template <typename T, bool is_pair>
struct IsTriviallyRelocatable {};

template <typename T>
struct IsTriviallyRelocatable<T, false> {
  static constexpr auto value =
      IsTrivial<T, is_instance<T, std::pair>{}>::value ||
      has_trivially_relocatable_flag<T>::value;
};

template <typename T>
struct IsTriviallyRelocatable<T, true> {
  static constexpr auto value =
      (IsTrivial<typename T::first_type,
                 is_instance<typename T::first_type, std::pair>{}>::value ||
       IsTriviallyRelocatable<
           typename T::first_type,
           is_instance<typename T::first_type, std::pair>{}>::value) &&
      (IsTrivial<typename T::second_type,
                 is_instance<typename T::second_type, std::pair>{}>::value ||
       IsTriviallyRelocatable<
           typename T::second_type,
           is_instance<typename T::second_type, std::pair>{}>::value);
};

template <class T>
void _nontrivial_destruct_reverse([[maybe_unused]] T* begin,
                                  [[maybe_unused]] size_t count) {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    // To be consistent with std::vector. Elements are destructed from back.
    auto end = begin + count;
    while (end != begin) {
      --end;
      end->~T();
    }
    //  for (; begin != end; ++begin) {
    //    begin->~T();
    //  }
  }
}

template <class T>
void _nontrivial_destruct_one(T* begin) {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    begin->~T();
  }
}

template <class T>
BASE_VECTOR_NEVER_INLINE T* _nontrivial_move(T* dest, T* source,
                                             ptrdiff_t count) {
  auto end = source + count;
  auto step = count >= 0 ? 1 : -1;
  for (; source != end; source += step, dest += step) {
    *dest = std::move(*source);
  }
  return dest;
}

template <class T>
BASE_VECTOR_INLINE T* _nontrivial_move_forward(T* dest, T* source,
                                               size_t count) {
  return _nontrivial_move<T>(dest, source, count);
}

template <class T>
BASE_VECTOR_INLINE T* _nontrivial_move_backward(T* dest, T* source,
                                                size_t count) {
  return _nontrivial_move<T>(dest, source, -count);
}

template <class T>
BASE_VECTOR_NEVER_INLINE void _nontrivial_construct_move(T* dest, T* source,
                                                         size_t count) {
  auto end = source + count;
  for (; source != end; ++source, ++dest) {
    ::new (static_cast<void*>(dest)) T(std::move(*source));
  }
}

template <class T>
BASE_VECTOR_NEVER_INLINE void _nontrivial_construct_copy(T* dest, T* source,
                                                         size_t count) {
  auto end = source + count;
  for (; source != end; ++source, ++dest) {
    ::new (static_cast<void*>(dest)) T(*source);
  }
}
}  // namespace

/**
 * Prototype of Vector provides view of its three members.
 * This base class does not provide any method to change data.
 */
template <class T>
struct VectorPrototype {
  static constexpr auto is_trivial =
      IsTrivial<T, is_instance<T, std::pair>{}>::value;
  static constexpr auto is_trivially_destructible =
      std::is_trivially_destructible_v<T>;
  static constexpr auto is_trivially_relocatable =
      IsTriviallyRelocatable<T, is_instance<T, std::pair>{}>::value;
  static constexpr auto is_trivially_destructible_after_move =
      is_trivially_relocatable ||
      IsTriviallyDestructibleAfterMove<T, is_instance<T, std::pair>{}>::value;

  using iterator = T*;
  using const_iterator = const T*;

  VectorPrototype() = default;

  [[maybe_unused]] size_t size() const { return count_; }

  bool empty() const { return count_ == 0; }

  size_t capacity() const { return std::abs(capacity_); }

  T* data() { return reinterpret_cast<T*>(_begin_iter()); }

  T* data() const { return reinterpret_cast<T*>(_begin_iter()); }

  /**
   * Returns if the array buffer is static or inplace buffer that should not be
   * freed.
   */
  bool is_static_buffer() const { return capacity_ < 0; }

 protected:
  template <class U>
  friend struct VectorPrototype;

  friend struct VectorTemplateless;

  BASE_VECTOR_NEVER_INLINE static void* _reallocate_buffer(void* array,
                                                           size_t element_size,
                                                           size_t count) {
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    auto old_capacity = proto_array->capacity();
    // old*2 for consistent with std::vector of libc++.
    // But we do not use max(count, old_cap * 2).
    auto new_capacity =
        count > 0
            ? count
            : (old_capacity == 0
                   ? (element_size >= 48
                          ? 1
                          : (element_size >= 16 ? 2 : (32 / element_size)))
                   : 2 * old_capacity);
    if (new_capacity <= old_capacity) {
      return nullptr;
    }
    const auto buffer_size = new_capacity * element_size;
    auto buffer = HeapAlloc(buffer_size);
    if (buffer == nullptr) {
      return nullptr;
    }
    proto_array->_set_memory(buffer);
    proto_array->capacity_ = static_cast<int32_t>(new_capacity);
    return buffer;
  }

  BASE_VECTOR_NEVER_INLINE void _reallocate_nontrivial(size_t count = 0) {
    static_assert(
        !is_trivial,
        "Only for non-trivial element types you can use this method.");
    auto prev_begin = _begin_iter();
    bool need_free = !is_static_buffer();
    auto buffer =
        VectorPrototype<uint8_t>::_reallocate_buffer(this, sizeof(T), count);
    if (buffer && prev_begin) {
      if constexpr (is_trivially_relocatable) {
        /* Fastest, allowing memcpy to copy all and skip destructors. */
        std::memcpy(buffer, prev_begin, sizeof(T) * size());
      } else {
        /* Move T objects one by one from old buffer to new buffer. */
        _nontrivial_construct_move((iterator)buffer, prev_begin, size());

        if constexpr (is_trivially_destructible_after_move) {
          /* Faster, skip destructors. */
        } else {
          /* Slow path, destruct objects on old buffer one by one. */
          _nontrivial_destruct_reverse(prev_begin, size());
        }
      }
      if (need_free) {
        HeapFree(prev_begin);
      }
    }
  }

  void _free() {
    if (memory_ != nullptr) {
      if constexpr (!is_trivial) {
        _nontrivial_destruct_reverse(_begin_iter(), size());
      }
      if (!is_static_buffer()) {
        HeapFree(_begin_iter());
      }
    }
  }

  void _reset() {
    memory_ = nullptr;
    count_ = 0;
    capacity_ = 0;
  }

  BASE_VECTOR_INLINE void* _memory() const { return memory_; }

  BASE_VECTOR_INLINE void _set_memory(void* value) {
    memory_ = reinterpret_cast<T*>(value);
  }

  BASE_VECTOR_INLINE iterator _begin_iter() const {
    return reinterpret_cast<iterator>(_memory());
  }

  BASE_VECTOR_INLINE iterator _end_iter() const {
    return _begin_iter() + count_;
  }

  BASE_VECTOR_INLINE iterator _finish_iter() const {
    return _begin_iter() + capacity();
  }

  BASE_VECTOR_INLINE void _set_count(size_t value) {
    count_ = static_cast<uint32_t>(value);
  }

  BASE_VECTOR_INLINE void _inc_count(size_t count = 1) {
    count_ += static_cast<uint32_t>(count);
  }

  BASE_VECTOR_INLINE void _dec_count(size_t count = 1) {
    count_ -= static_cast<uint32_t>(count);
  }

  /**
   * @brief Set capacity value by sub-classes.
   * @param value Capacity value.
   * @param static_buffer If true, capacity value will be stored as negative and
   *   the buffer will not be freed in reallocation or destructor.
   */
  BASE_VECTOR_INLINE void _set_capacity(size_t value, bool static_buffer) {
    capacity_ = static_buffer ? -static_cast<int32_t>(value)
                              : static_cast<int32_t>(value);
  }

  BASE_VECTOR_INLINE void _transfer_from(VectorPrototype& other) {
    // Memory buffer of other array might be an inplace buffer which is
    // unsafe to be directly transferred ownership to self.
    BASE_VECTOR_DCHECK(!other.is_static_buffer());

    memory_ = other.memory_;
    count_ = other.count_;
    capacity_ = other.capacity_;
  }

  template <class U>
  BASE_VECTOR_INLINE void _transfer_as_byte_array_from(
      VectorPrototype<U>& other) {
    // Memory buffer of other array might be an inplace buffer which is
    // unsafe to be directly transferred ownership to self.
    BASE_VECTOR_DCHECK(!other.is_static_buffer());

    memory_ = reinterpret_cast<T*>(other.memory_);
    count_ = static_cast<uint32_t>(other.count_ * sizeof(U));
    capacity_ = static_cast<int32_t>(other.capacity_ * sizeof(U));
  }

  void _swap(VectorPrototype& other) {
    // Memory buffer of other array might be an inplace buffer which is
    // unsafe to be directly transferred ownership to self.
    BASE_VECTOR_DCHECK(!is_static_buffer() && !other.is_static_buffer());

    std::swap(memory_, other.memory_);
    std::swap(count_, other.count_);
    std::swap(capacity_, other.capacity_);
  }

 private:
  T* memory_{nullptr};

  uint32_t count_{0};

  // Negative capacity value means the memory_ buffer should not be freed.
  int32_t capacity_{0};
};

/**
 APIs to manipulate Vector without template argument T.
 Note that templateless manipulators should only be used for trivial T types.
*/
struct VectorTemplateless {
  BASE_VECTOR_NEVER_INLINE static void ReallocateTrivial(void* array,
                                                         size_t element_size,
                                                         size_t count) {
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    /**
     Vector is also used for ECS dynamic buffer component storage.
     Buffer may be initially in chunk and the capacity_ value is negative
     to indicate that the memory should not be freed.
     */
    const bool free_prev = !proto_array->is_static_buffer();
    auto prev_begin = proto_array->_begin_iter();
    auto buffer = VectorPrototype<uint8_t>::_reallocate_buffer(
        array, element_size, count);
    if (buffer && prev_begin) {
      std::memcpy(buffer, prev_begin, proto_array->size() * element_size);
      if (free_prev) {
        HeapFree(prev_begin);
      }
    }
  }

  /**
   Use this never-inline function to replace ReallocateTrivial full arguments
   version to avoid one default-to-0 argument at caller site. This function
   passes count==0 to grow the capacity.
   */
  BASE_VECTOR_NEVER_INLINE static void ReallocateTrivial(void* array,
                                                         size_t element_size) {
    ReallocateTrivial(array, element_size, 0);
  }

  /**
   All trivial arrays share the same Resize implementation for binary size
   benefits. Note that filling operations are done by std::memcpy which may be
   slower than std::vector.
   */
  BASE_VECTOR_NEVER_INLINE static bool Resize(void* array, size_t element_size,
                                              size_t count,
                                              const void* fill_value) {
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    bool reallocated = false;
    if (count > proto_array->size()) {
      if (count > proto_array->capacity()) {
        ReallocateTrivial(array, element_size, count);
        reallocated = true;
      }
      if (fill_value) {
        auto begin = proto_array->_begin_iter();
        for (size_t i = proto_array->size(); i < count; i++) {
          std::memcpy(begin + i * element_size, fill_value, element_size);
        }
      }
    }
    proto_array->_set_count(count);
    return reallocated;
  }

  /**
   This function prepares space for the data to be inserted, but does not
   perform the final insertion.
   */
  BASE_VECTOR_NEVER_INLINE static void* PrepareInsert(void* array,
                                                      size_t element_size,
                                                      const void* dest) {
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    auto diff = static_cast<const uint8_t*>(dest) - proto_array->_begin_iter();
    if (proto_array->size() == proto_array->capacity()) {
      ReallocateTrivial(array, element_size);
    }
    auto pos = proto_array->_begin_iter() + diff;
    auto new_last =
        proto_array->_begin_iter() + proto_array->size() * element_size;
    if (pos > new_last) {
      BASE_VECTOR_DCHECK(false);
    } else {
      if (pos < new_last) {
        std::memmove(pos + element_size, pos, new_last - pos);
      }
      // The actual assignment is performed after PrepareInsert() so that memcpy
      // could be optimized by compiler. std::memcpy(pos, source, element_size);
      proto_array->_inc_count();
    }
    return pos;
  }

  /**
   Erase element at index. The index is measured by element_size.
   Returns false if index or index + count is out of range.
   */
  BASE_VECTOR_NEVER_INLINE static bool Erase(void* array, size_t element_size,
                                             size_t index, size_t count = 1) {
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    if (index >= proto_array->size()) {
      return false;
    }
    if (index + count > proto_array->size()) {
      return false;
    }
    auto dst = proto_array->_begin_iter() + element_size * index;
    std::memmove(dst, dst + element_size * count,
                 element_size * (proto_array->size() - index - count));
    proto_array->_set_count(proto_array->size() - count);
    return true;
  }

  /**
   Push N[count] elements from [source] to Vector of address [array].
   */
  BASE_VECTOR_NEVER_INLINE static void PushBackBatch(void* array,
                                                     size_t element_size,
                                                     const void* source,
                                                     size_t count) {
    if (source == nullptr) {
      return;
    }
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    if (proto_array->size() + count > proto_array->capacity()) {
      ReallocateTrivial(array, element_size, proto_array->size() + count);
    }
    auto end = proto_array->_begin_iter() + element_size * proto_array->size();
    std::memcpy(end, source, count * element_size);
    proto_array->_inc_count(count);
  }

  /**
   Fill elements from index position of original array with contents of
   [source].
   */
  BASE_VECTOR_NEVER_INLINE static void Fill(void* array, size_t element_size,
                                            const void* source,
                                            size_t byte_size, size_t position) {
    const auto source_count = byte_size / element_size;
    if (source_count == 0) {
      return;
    }
    VectorPrototype<uint8_t>* proto_array =
        reinterpret_cast<VectorPrototype<uint8_t>*>(array);
    auto count = source_count + position;
    ReallocateTrivial(array, element_size, count);
    auto dest = proto_array->_begin_iter() + position * element_size;
    if (source != nullptr) {
      std::memcpy(dest, source, source_count * element_size);
    } else {
      std::memset(dest, 0, source_count * element_size);
    }
    proto_array->_set_count(count);
  }
};

/**
 * Replacement of Vector for binary size benefits. This linear container
 * provides basic methods of Vector with the same method signatures. For
 * POD types, it also provides some non-stl-standard methods for convenience.
 */
template <class T>
struct Vector : protected VectorTemplateless, public VectorPrototype<T> {
  using typename VectorPrototype<T>::iterator;
  using typename VectorPrototype<T>::const_iterator;
  using VectorPrototype<T>::is_trivial;
  using VectorPrototype<T>::is_trivially_destructible;
  using VectorPrototype<T>::is_trivially_destructible_after_move;
  using VectorPrototype<T>::is_trivially_relocatable;
  using VectorPrototype<T>::size;
  using VectorPrototype<T>::capacity;
  using VectorPrototype<T>::is_static_buffer;
  using VectorPrototype<T>::_begin_iter;
  using VectorPrototype<T>::_end_iter;
  using VectorPrototype<T>::_finish_iter;
  using VectorPrototype<T>::_free;
  using VectorPrototype<T>::_transfer_from;
  using VectorPrototype<T>::_transfer_as_byte_array_from;
  using VectorPrototype<T>::_swap;
  using VectorPrototype<T>::_reset;
  using VectorPrototype<T>::_set_count;
  using VectorPrototype<T>::_inc_count;
  using VectorPrototype<T>::_dec_count;
  using VectorPrototype<T>::_reallocate_nontrivial;

  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using unique_id_type = uintptr_t;

  /**
   * @brief We allows 'array = nullptr' to reset and clear memory of array.
   */
  Vector(std::nullptr_t) {}  // NOLINT
  Vector() = default;
  explicit Vector(size_t count) { _construct_fill_default(count); }

  Vector(size_t count, const T& value) { _construct_fill(count, value); }

  /**
   * @brief Create array by size and data, if parameter 'data' is valid, copy it
   * to array.
   * @param count element count of array
   * @param data source data. If non-null, will copy from data.
   */
  template <class U = T, typename = typename std::enable_if<
                             std::is_same_v<U, T> && is_trivial>::type>
  Vector(size_t count, const void* data) {
    fill(data, count * sizeof(T));
  }

  Vector(std::initializer_list<T> list) {
    _from(list.begin(), list.size(), _nontrivial_construct_move);
  }

  /**
   * @brief We only support construct array from array iterators because we can
   * calculate distance from begin to end. std::vector supports being
   * constructed from other iterators such as std::set's, but it is not
   * efficient because we cannot precalculate capacity of the array.
   */
  Vector(const_iterator begin, const_iterator end) {
    _from(begin, end - begin, _nontrivial_construct_copy);
  }

  Vector(iterator begin, iterator end) {
    _from(begin, end - begin, _nontrivial_construct_copy);
  }

  Vector(const Vector& other) {
    _from(other.data(), other.size(), _nontrivial_construct_copy);
  }

  Vector& operator=(const Vector& other) {
    if (this != &other) {
      clear();
      _from(other.data(), other.size(), _nontrivial_construct_copy);
    }
    return *this;
  }

  Vector& operator=(std::initializer_list<T> list) {
    clear();
    _from(list.begin(), list.size(), _nontrivial_construct_move);
    return *this;
  }

  Vector(Vector&& other) {
    if (other.is_static_buffer()) {
      _from(other.data(), other.size(), _nontrivial_construct_move);
      other.clear();
    } else {
      _transfer_from(other);
      other._reset();
    }
  }

  Vector& operator=(Vector&& other) {
    if (this != &other) {
      if (other.is_static_buffer()) {
        clear();
        _from(other.data(), other.size(), _nontrivial_construct_move);
        other.clear();
      } else {
        _free();
        _transfer_from(other);
        other._reset();
      }
    }
    return *this;
  }

  ~Vector() { _free(); }

  template <class U>
  reference push_back(U&& v) {
    return emplace_back(std::forward<U>(v));
  }

  template <class... Args>
  reference emplace_back(Args&&... args) {
    _grow_if_need();
    auto end = _end_iter();
    ::new (static_cast<void*>(end)) T(std::forward<Args>(args)...);
    _inc_count();
    return *end;
  }

  void pop_back() {
    if (size() == 0) {
      return;
    }
    if constexpr (!is_trivial) {
      _nontrivial_destruct_one(_end_iter() - 1);
    }
    _dec_count();
  }

  const_reference back() const { return *(_end_iter() - 1); }

  reference back() { return *(_end_iter() - 1); }

  const_reference front() const { return *_begin_iter(); }

  reference front() { return *_begin_iter(); }

  const_reference operator[](size_t n) const {
    BASE_VECTOR_DCHECK(n < size());
    return *(_begin_iter() + n);
  }

  reference operator[](size_t n) {
    BASE_VECTOR_DCHECK(n < size());
    return *(_begin_iter() + n);
  }

  const_reference at(size_t n) const {
    BASE_VECTOR_DCHECK(n < size());
    return *(_begin_iter() + n);
  }  // Always nothrow

  reference at(size_t n) {
    BASE_VECTOR_DCHECK(n < size());
    return *(_begin_iter() + n);
  }  // Always nothrow

  /**
   * @brief Returns pointer to the underlying array serving as element storage.
   * @return If size() is ​0​, data() may or may not return a null pointer.
   */
  template <typename U = T>
  U* data() {
    return reinterpret_cast<U*>(_begin_iter());
  }

  /**
   * @brief Returns pointer to the underlying array serving as element storage.
   * @return If size() is ​0​, data() may or may not return a null pointer.
   */
  template <typename U = T>
  const U* data() const {
    return reinterpret_cast<const U*>(_begin_iter());
  }

  iterator begin() { return _begin_iter(); }

  const_iterator begin() const { return _begin_iter(); }

  const_iterator cbegin() const { return _begin_iter(); }

  iterator end() { return _end_iter(); }

  const_iterator end() const { return _end_iter(); }

  const_iterator cend() const { return _end_iter(); }

  reverse_iterator rbegin() { return reverse_iterator(_end_iter()); }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(_end_iter());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(_end_iter());
  }

  reverse_iterator rend() { return reverse_iterator(_begin_iter()); }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(_begin_iter());
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(_begin_iter());
  }

  iterator erase(std::nullptr_t) = delete;  // Avoid misuse of erase(0)
  iterator erase(std::nullptr_t,
                 std::nullptr_t) = delete;  // Avoid misuse of erase(0)
  iterator erase(iterator pos) { return erase(pos, pos + 1); }

  const_iterator erase(const_iterator pos) {
    return (const_iterator)erase((iterator)pos);
  }

  iterator erase(iterator first, iterator last) {
    BASE_VECTOR_DCHECK(first <= last);
    if (first != last) {
      if constexpr (is_trivial || is_trivially_relocatable) {
        std::memmove(first, last, (_end_iter() - last) * sizeof(T));
      } else {
        // Slow path, move elements one by one and skip destructors if possible.
        [[maybe_unused]] auto it =
            _nontrivial_move_forward(first, last, _end_iter() - last);
        if constexpr (!is_trivially_destructible_after_move) {
          _nontrivial_destruct_reverse(it, last - first);
        }
      }
      _dec_count(last - first);
    }
    return first;
  }

  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    if constexpr (is_trivial) {
      // For trivial types, always call templateless implementation for binary
      // size benefits.
      auto it = (iterator)PrepareInsert(this, sizeof(T), pos);
      ::new (static_cast<void*>(it)) T(std::forward<Args>(args)...);
      return it;
    } else {
      auto pair = _nontrivial_prepare_insert(pos);
      if constexpr (!is_trivially_destructible) {
        if constexpr (!is_trivially_relocatable) {
          // If T is_trivially_relocatable, we can skip destructor because in
          // _nontrivial_prepare_insert the element at `pair.first` is moved
          // away by `memmove`.
          if (!pair.second) {
            // Element at dest_pos was moved but needs destruction.
            pair.first->~T();
          }
        }
      }
      // Construct at dest_pos.
      ::new (static_cast<void*>(pair.first)) T(std::forward<Args>(args)...);
      return pair.first;
    }
  }

  template <class U>
  iterator insert(std::nullptr_t,
                  U&& value) = delete;  // Avoid misuse of insert(0)

  BASE_VECTOR_INLINE iterator insert(const_iterator pos, const T& value) {
    return emplace(pos, value);
  }

  BASE_VECTOR_INLINE iterator insert(const_iterator pos, T&& value) {
    return emplace(pos, std::move(value));
  }

  /**
   * @brief Reserve capacity.
   * @return True if reallocation occurs.
   */
  bool reserve(size_t count) {
    if (count > capacity()) {
      _reallocate(count);
      return true;
    }
    return false;
  }

  void clear() {
    if constexpr (!is_trivial) {
      _nontrivial_destruct_reverse(_begin_iter(), size());
    }
    // The capacity should not change.
    _set_count(0);
  }

  void clear_and_shrink() {
    _free();
    _reset();
  }

  void shrink_to_fit() {
    if (is_static_buffer()) {
      return;
    }
    if (capacity() > size()) {
      // This is not optimal for InlineVector if current size fits into inline
      // buffer.
      Vector tmp;
      tmp._from(data(), size(), _nontrivial_construct_move);
      _swap(tmp);
    }
  }

  void swap(Vector& other) {
    if (is_static_buffer() || other.is_static_buffer()) {
      // If any one is using inline static buffer, we cannot simply swap the
      // pointers.
      Vector tmp(*this);
      *this = other;
      other = tmp;
    } else {
      _swap(other);
    }
  }

  /**
   * @brief Resize the array to desired length.
   *  template<bool fill>:  If you want to set different values for new elements
   *  after resize, you can pass fill as false to wipe out unnecessary
   * assignment operations. This template argument is required for performance
   * benefits.
   *
   * @param count Desired length.
   * @return True if reallocation occurs.
   */
  template <bool fill, class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial && fill,
                          bool>::type
  resize(size_t count) {
    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                  std::is_pointer_v<T>) {
      // For efficiency, merge to single memset instead of std::memcpy in
      // Resize.
      bool reallocated = false;
      if (count > size()) {
        if (count > capacity()) {
          ReallocateTrivial(this, sizeof(T), count);
          reallocated = true;
        }
        std::memset(begin() + size(), 0, (count - size()) * sizeof(T));
      }
      _set_count(count);
      return reallocated;
    } else {
      if (count > size()) {
        T value;
        return Resize(this, sizeof(T), count, &value);
      } else {
        _set_count(count);
        return false;
      }
    }
  }

  template <bool fill, class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial && fill,
                          bool>::type
  resize(size_t count, const T& value) {
    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                  std::is_pointer_v<T>) {
      // For efficiency, merge to single memset if value is T() instead of
      // std::memcpy in Resize.
      bool reallocated = false;
      if (count > size()) {
        if (count > capacity()) {
          ReallocateTrivial(this, sizeof(T), count);
          reallocated = true;
        }
        if (value == T()) {
          std::memset(begin() + size(), 0, (count - size()) * sizeof(T));
        } else {
          std::fill(begin() + size(), begin() + count, value);
        }
      }
      _set_count(count);
      return reallocated;
    } else {
      return Resize(this, sizeof(T), count, &value);
    }
  }

  template <bool fill, class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial && !fill,
                          bool>::type
  resize(size_t count) {
    return Resize(this, sizeof(T), count, nullptr);
  }

  /**
   * \brief It is recommended to call grow(N) for nontrivial type if you are
   * surely to expand the array. Because resize() method specialize a branch to
   * erase items.
   */
  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && !is_trivial &&
                              std::is_copy_constructible_v<U>,
                          bool>::type
  resize(size_t count, const T& value) {
    bool reallocated = false;
    if (count > size()) {
      if (count > capacity()) {
        _reallocate(count);
        reallocated = true;
      }
      _fill(_end_iter(), _begin_iter() + count, value);
      _set_count(static_cast<uint32_t>(count));
    } else {
      erase(_begin_iter() + count, _end_iter());
    }
    return reallocated;
  }

  /**
   * \brief It is recommended to call grow(N) for nontrivial type if you are
   * surely to expand the array. Because resize() method specialize a branch to
   * erase items.
   */
  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && !is_trivial, bool>::type
  resize(size_t count) {
    bool reallocated = false;
    if (count > size()) {
      if (count > capacity()) {
        _reallocate(count);
        reallocated = true;
      }
      _fill(_end_iter(), _begin_iter() + count);
      _set_count(static_cast<uint32_t>(count));
    } else {
      erase(_begin_iter() + count, _end_iter());
    }
    return reallocated;
  }

  /**
   * \brief Grow by increment length and returns the last object.
   * The difference between grow and push_back/emplace_back is that for trivial
   * type grow method does not do any copy or construction. For non-trivial
   * type, grow only construct by default.
   */
  reference grow() {
    _grow_if_need();
    if constexpr (!is_trivial) {
      auto end = _end_iter();
      ::new (static_cast<void*>(end)) T();
      _inc_count();
      return *end;
    } else {
      _inc_count();
      return back();
    }
  }

  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial>::type grow(
      size_t count) {
    BASE_VECTOR_DCHECK(count >= size());
    resize<false>(count);
  }

  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && !is_trivial>::type grow(
      size_t count) {
    if (count > size()) {
      if (count > capacity()) {
        _reallocate(count);
      }
      _fill(_end_iter(), _begin_iter() + count);
      _set_count(static_cast<uint32_t>(count));
    } else if (count < size()) {
      BASE_VECTOR_DCHECK(false);
    }
  }

  /**
   * @brief Use data to fill array buffer. The size of array will be reset to
   *  byte_size / sizeof(T) + position.
   *
   * @param data Data source pointer. If null, buffer will be filled by 0.
   * @param byte_size Data source byte length.
   * @param position Optional, from which index of T to write.
   */
  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial>::type fill(
      const void* data, size_type byte_size, size_type position = 0) {
    VectorTemplateless::Fill(this, sizeof(T), data, byte_size, position);
  }

  /**
   * @brief Append data buffer to end of this array.
   * @param data Data source pointer. If null, buffer will be filled by 0.
   * @param byte_size Data source byte length.
   */
  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial>::type append(
      const void* data, size_type byte_size) {
    fill(data, byte_size, size());
  }

  /**
   * @brief Append data buffer of another array to end of this array.
   * @param other Another array which may differ in element type.
   */
  template <class W = T, class U>
  typename std::enable_if<std::is_same_v<W, T> && is_trivial>::type append(
      const Vector<U>& other) {
    if (!other.empty()) {
      fill(other.data(), other.size() * sizeof(U), size());
    }
  }

  /**
   * @brief Transfer buffer to result array and set length of result array to
   * byte length of data.
   */
  template <class U = T>
  typename std::enable_if<std::is_same_v<U, T> && is_trivial && (sizeof(T) > 1),
                          Vector<uint8_t>>::type
  transfer_to_byte_array() {
    Vector<uint8_t> result;
    result._transfer_as_byte_array_from<T>(*this);
    _reset();
    return result;
  }

  /**
   * @brief Sugar for_each method which provides only T or T& in callback.
   */
  template <typename Callback>
  void for_each(Callback&& callback) {
    const auto count = size();
    for (uint32_t i = 0; i < count; i++) {
      callback((*this)[i]);
    }
  }

 protected:
  template <class U>
  struct always_false : std::false_type {};

  void _reallocate(size_t value) {
    if constexpr (is_trivial) {
      ReallocateTrivial(this, sizeof(T), value);
    } else {
      _reallocate_nontrivial(value);
    }
  }

  void _grow_if_need() {
    if (size() == capacity()) {
      // _grow_if_need is inlined at caller side. Although we can call
      // _reallocate(0) but implement standalone for binary size optimization.
      if constexpr (is_trivial) {
        ReallocateTrivial(this, sizeof(T));
      } else {
        _reallocate_nontrivial();
      }
    }
  }

  void _from(const void* src, size_t size,
             [[maybe_unused]] void (*func)(iterator, iterator, size_t)) {
    if (size > 0) {
      if constexpr (is_trivial) {
        PushBackBatch(this, sizeof(T), src, size);
      } else {
        reserve(size);
        func(_begin_iter(), reinterpret_cast<iterator>(const_cast<void*>(src)),
             size);
        _set_count(static_cast<uint32_t>(size));
      }
    }
  }

  void _fill(iterator begin, iterator end, const T& v) {
    for (; begin != end; begin++) {
      ::new (static_cast<void*>(begin)) T(v);
    }
  }

  void _fill(iterator begin, iterator end) {
    for (; begin != end; begin++) {
      ::new (static_cast<void*>(begin)) T();
    }
  }

  void _construct_fill_default(size_t count) {
    if (count > 0) {
      if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                    std::is_pointer_v<T>) {
        fill(nullptr, count * sizeof(T));  // Resize and set all bits to 0.
      } else if constexpr (is_trivial) {
        resize<true>(count);
      } else {
        // Do not use `resize` directly because for non-trivial types, the
        // `resize` method instantiates `erase` method.
        _reallocate(count);
        _fill(_end_iter(), _begin_iter() + count);
        _set_count(static_cast<uint32_t>(count));
      }
    }
  }

  void _construct_fill(size_t count, const T& value) {
    if (count > 0) {
      if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> ||
                    std::is_pointer_v<T>) {
        if (value == T()) {
          fill(nullptr, count * sizeof(T));  // Resize and set all bits to 0.
        } else {
          resize<false>(count);
          std::fill(begin(), end(), value);
        }
      } else if constexpr (is_trivial) {
        resize<true>(count, value);
      } else {
        // Do not use `resize` directly because for non-trivial types, the
        // `resize` method instantiates `erase` method.
        _reallocate(count);
        _fill(_end_iter(), _begin_iter() + count, value);
        _set_count(static_cast<uint32_t>(count));
      }
    }
  }

  // Returns pair<pos, true_if_construct_at_end>
  BASE_VECTOR_INLINE std::pair<iterator, bool> _nontrivial_prepare_insert(
      const_iterator pos) {
    auto diff = pos - _begin_iter();
    _grow_if_need();
    auto dest_pos = _begin_iter() + diff;
    auto new_last = _end_iter();
    bool construct_at_end = false;
    if (dest_pos > new_last) {
      BASE_VECTOR_DCHECK(false);
    } else {
      if (dest_pos < new_last) {
        if constexpr (is_trivially_relocatable) {
          // Fast path, use memmove to shift all elements to next slot.
          std::memmove(dest_pos + 1, dest_pos,
                       sizeof(T) * (new_last - dest_pos));
        } else {
          // Slow path, construct new T at end, move previous back item to it.
          _nontrivial_construct_move(new_last, new_last - 1, 1);
          // Move left objects.
          _nontrivial_move_backward(new_last - 1, new_last - 2,
                                    new_last - dest_pos - 1);
        }
      } else {
        construct_at_end = true;
      }
      _inc_count();
    }
    return {dest_pos, construct_at_end};
  }
};

template <class T>
inline bool operator==(const Vector<T>& __x, const Vector<T>& __y) {
  const typename Vector<T>::size_type __sz = __x.size();
  return __sz == __y.size() && std::equal(__x.begin(), __x.end(), __y.begin());
}

template <class T>
inline bool operator!=(const Vector<T>& __x, const Vector<T>& __y) {
  return !(__x == __y);
}

template <class T>
inline bool operator<(const Vector<T>& __x, const Vector<T>& __y) {
  return std::lexicographical_compare(__x.begin(), __x.end(), __y.begin(),
                                      __y.end());
}

template <class T>
inline bool operator>(const Vector<T>& __x, const Vector<T>& __y) {
  return __y < __x;
}

template <class T>
inline bool operator>=(const Vector<T>& __x, const Vector<T>& __y) {
  return !(__x < __y);
}

template <class T>
inline bool operator<=(const Vector<T>& __x, const Vector<T>& __y) {
  return !(__y < __x);
}

using ByteArray = Vector<uint8_t>;

/**
 * @brief Sugar for construction ByteArray from static primitive array.
 * ByteArray a = ByteArrayFromBuffer((float[]) {0, 1, 1, 1, 1, 0, 0, 0});
 */
template <typename T, size_t Num>
ByteArray ByteArrayFromBuffer(const T (&data)[Num]) {
  return ByteArray(sizeof(data), data);
}

/**
 * @brief A resizable array type initialized with capacity of N and the buffer
 * is inplace following up the array struct. When element count exceeds N, a new
 * external buffer will be allocated and the inplace_buffer_ will be wasted.
 */
template <class T, size_t N>
struct InlineVector : public Vector<T> {
  using typename VectorPrototype<T>::iterator;
  using typename VectorPrototype<T>::const_iterator;
  using VectorPrototype<T>::is_trivial;
  using VectorPrototype<T>::size;
  using VectorPrototype<T>::capacity;
  using VectorPrototype<T>::_set_memory;
  using VectorPrototype<T>::_set_capacity;
  using Vector<T>::_begin_iter;
  using Vector<T>::_end_iter;
  using Vector<T>::reserve;
  using Vector<T>::clear;
  using Vector<T>::clear_and_shrink;
  using Vector<T>::_from;
  using Vector<T>::_construct_fill;
  using Vector<T>::_construct_fill_default;

  static constexpr size_t kInlinedSize = N;

  InlineVector() : Vector<T>() {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
  }

  InlineVector(std::nullptr_t) : InlineVector() {}  // NOLINT

  explicit InlineVector(size_t count) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    _construct_fill_default(count);
  }

  InlineVector(size_t count, const T& value) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    _construct_fill(count, value);
  }

  InlineVector(const_iterator begin, const_iterator end) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    _from(begin, end - begin, _nontrivial_construct_copy);
  }

  InlineVector(iterator begin, iterator end) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    _from(begin, end - begin, _nontrivial_construct_copy);
  }

  InlineVector(std::initializer_list<T> list) : Vector<T>() {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = std::move(list);
  }

  InlineVector(const Vector<T>& other) {  // NOLINT
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = other;
  }

  InlineVector(const InlineVector& other) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = other;
  }

  template <size_t N2>
  InlineVector(const InlineVector<T, N2>& other) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = other;
  }

  InlineVector(Vector<T>&& other) {  // NOLINT
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = std::move(other);
  }

  InlineVector(InlineVector&& other) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = std::move(other);
  }

  template <size_t N2>
  InlineVector(InlineVector<T, N2>&& other) {
    static_assert(N > 0,
                  "InlineVector must have initial capacity larger than 0.");
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
    *this = std::move(other);
  }

  InlineVector& operator=(std::initializer_list<T> list) {
    reinterpret_cast<Vector<T>*>(this)->operator=(std::move(list));
    return *this;
  }

  InlineVector& operator=(const Vector<T>& other) {
    reinterpret_cast<Vector<T>*>(this)->operator=(other);
    return *this;
  }

  InlineVector& operator=(const InlineVector& other) {
    reinterpret_cast<Vector<T>*>(this)->operator=(other);
    return *this;
  }

  template <size_t N2>
  InlineVector& operator=(const InlineVector<T, N2>& other) {
    reinterpret_cast<Vector<T>*>(this)->operator=(other);
    return *this;
  }

  InlineVector& operator=(Vector<T>&& other) {
    if (this != &other) {
      if (other.size() > capacity()) {
        if (!other.is_static_buffer()) {
          // Just swap pointers.
          reinterpret_cast<Vector<T>*>(this)->operator=(std::move(other));
          return *this;
        } else {
          reserve(other.size());
        }
      }

      // Trying best to use inplace buffer, move items of other to self.
      clear();
      _from(other.begin(), other.size(), _nontrivial_construct_move);
      other.clear();
    }
    return *this;
  }

  InlineVector& operator=(InlineVector&& other) {
    return operator=(std::move(*reinterpret_cast<Vector<T>*>(&other)));
  }

  template <size_t N2>
  InlineVector& operator=(InlineVector<T, N2>&& other) {
    return operator=(std::move(*reinterpret_cast<Vector<T>*>(&other)));
  }

  void clear_and_shrink() {
    Vector<T>::clear_and_shrink();
    _set_capacity(N, true);
    _set_memory(&inplace_buffer_[0]);
  }

 private:
  alignas(std::max(alignof(T),
                   sizeof(void*))) uint8_t inplace_buffer_[sizeof(T) * N];
};

/**
 * @brief Stack using Vector<T> or InlineVector<T, N> as underlying container.
 */
template <class T>
using Stack = std::stack<T, Vector<T>>;

template <class T, size_t N>
using InlineStack = std::stack<T, InlineVector<T, N>>;

template <bool Flag>
struct MapStatisticsBase;

enum class MapStatisticsFindKind {
  kFind,                 // find(), contains() or [] to find a value
  kInsertFindCollision,  // key found before insert
  kInsertFind,           // key not found before insert
};

template <>
struct MapStatisticsBase<true> {
  mutable uint32_t max_count{0};
  mutable uint32_t insert_count{0};
  mutable uint32_t erase_count{0};
  mutable uint32_t total_find_count{0};
  mutable uint32_t find_count{0};
  mutable uint32_t insert_find_collision_count{0};

  ~MapStatisticsBase();

  void UpdateMaxCount(size_t v) const;

  void IncreaseInsertCount() const { insert_count++; }

  void IncreaseEraseCount() const { erase_count++; }

  void RecordFind(MapStatisticsFindKind kind, size_t find_of_count) const;

  static void IncreaseFindOfCount(MapStatisticsFindKind kind,
                                  size_t find_of_count);
};

template <>
struct MapStatisticsBase<false> {
  void UpdateMaxCount([[maybe_unused]] size_t v) const {}
  void IncreaseInsertCount() const {}
  void IncreaseEraseCount() const {}
  void RecordFind([[maybe_unused]] MapStatisticsFindKind kind,
                  [[maybe_unused]] size_t find_of_count) const {}
};

/**
 * @brief Base storage type for binary-search array and linear search array.
 * It provides definitions and interfaces which have nothing to do with
 * query method of key.
 */
template <class K, class T, size_t N, bool Stat>
struct KeyValueArray : protected MapStatisticsBase<Stat> {
  static constexpr auto is_map = !std::is_void_v<T>;

  using key_type = K;
  using mapped_type = T;
  using value_type =
      typename std::conditional_t<is_map, std::pair<const K, T>, K>;
  using container_type =
      typename std::conditional_t<(N > 0), InlineVector<value_type, N>,
                                  Vector<value_type>>;

 protected:
  // Storing pair<K, T> with non-const K for assignment.
  using store_value_type =
      typename std::conditional_t<is_map, std::pair<K, T>, K>;
  using store_container_type =
      typename std::conditional_t<(N > 0), InlineVector<store_value_type, N>,
                                  Vector<store_value_type>>;
  using store_iterator =
      std::conditional_t<is_map, typename store_container_type::iterator,
                         typename store_container_type::const_iterator>;

  container_type& array() const {
    return *reinterpret_cast<container_type*>(
        const_cast<store_container_type*>(&array_));
  }

  template <class K2, class T2, size_t N2, bool Stat2>
  friend struct KeyValueArray;

  using MapStatisticsBase<Stat>::UpdateMaxCount;
  using MapStatisticsBase<Stat>::IncreaseEraseCount;

 public:
  using iterator = std::conditional_t<is_map, typename container_type::iterator,
                                      typename container_type::const_iterator>;
  using const_iterator = typename container_type::const_iterator;
  using reverse_iterator =
      std::conditional_t<is_map, typename container_type::reverse_iterator,
                         typename container_type::const_reverse_iterator>;
  using const_reverse_iterator =
      typename container_type::const_reverse_iterator;

 public:
  KeyValueArray() = default;

  KeyValueArray(Vector<store_value_type>&& source_array)
      : array_(std::move(source_array)) {
    UpdateMaxCount(array_.size());
  }

  template <size_t N2>
  KeyValueArray(const KeyValueArray<K, T, N2, Stat>& other)
      : array_(other.array_) {
    UpdateMaxCount(array_.size());
  }

  template <size_t N2>
  KeyValueArray(KeyValueArray<K, T, N2, Stat>&& other)
      : array_(std::move(other.array_)) {
    UpdateMaxCount(array_.size());
  }

  template <size_t N2>
  KeyValueArray& operator=(const KeyValueArray<K, T, N2, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  KeyValueArray& operator=(KeyValueArray<K, T, N2, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  bool is_static_buffer() const { return array_.is_static_buffer(); }

  size_t size() const { return array_.size(); }

  bool empty() const { return array_.empty(); }

  void clear() {
    // We should also free memory so `array_.clear()` is not feasible.
    array_.clear_and_shrink();
  }
  void clear_keep_buffer() { array_.clear(); }

  bool reserve(size_t count) { return array_.reserve(count); }

  iterator begin() { return array().begin(); }

  const_iterator begin() const { return array().begin(); }

  const_iterator cbegin() const { return array().cbegin(); }

  iterator end() { return array().end(); }

  const_iterator cend() const { return array().cend(); }

  const_iterator end() const { return array().end(); }

  reverse_iterator rbegin() { return array().rbegin(); }

  const_reverse_iterator rbegin() const { return array().rbegin(); }

  const_reverse_iterator crbegin() const { return array().crbegin(); }

  reverse_iterator rend() { return array().rend(); }

  const_reverse_iterator rend() const { return array().rend(); }

  const_reverse_iterator crend() const { return array().crend(); }

  iterator erase(iterator pos) {
    IncreaseEraseCount();
    return reinterpret_cast<iterator>(
        array_.erase(reinterpret_cast<store_iterator>(pos)));
  }

  template <size_t N2>
  bool operator==(const KeyValueArray<K, T, N2, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const KeyValueArray<K, T, N2, Stat>& other) const {
    return array_ != other.array_;
  }

 protected:
  store_container_type array_;

  template <size_t N2>
  void _swap(KeyValueArray<K, T, N2, Stat>& other) {
    array_.swap(other.array_);
  }
};

/**
 * @brief For ordered map/set base on array. Performance consideration:
 *  1. Insertion is relatively slow because of reallocation and moving of
 * elements. While map/set also need rebalancing of RB tree.
 * Performance test data shows that when the key is a basic type such as int,
 * the insertion performance of the container is still ahead of std::map when
 * the data volume reaches several thousands. When the key is a type with
 * relatively low movement overhead such as std::string, the insertion
 * performance is on par with std::map when the data volume reaches 30-50.
 * Besides, ordered map/set on array supports real reserve() to pre-allocate
 * container buffer.
 *  2. Finding is array-based binary-search which should outperform map/set
 * because array is linear and much simpler.
 *  3. Memory layout is much better than node-based map and unordered_map
 * because elements are stored linearly and continuously.
 *  4. Compiled binary size is the same as map/set and smaller than
 * unordered_map/unordered_set.
 *  5. Struct type size(16b) is smaller than std::map, std::set,
 * std::unordered_map and std::unordered_set.
 *
 * Best use scenarios:
 * 1. Small amount of data, from dozens to hundreds.
 * 2. Low moving overhead for Key and Value, such as primitive types,
 * std::string, std::shared_ptr.
 * 3. More queries than data modifications.
 *
 * Please use the following specialized types:
 *   OrderedFlatMap
 *   InlineOrderedFlatMap
 *   OrderedFlatSet
 *   InlineOrderedFlatSet
 *
 * When implementing custom Compare method. Use f(const K&, const K&) as
 * signature to avoid unnecessary copy.
 */
template <class K, class T, size_t N, class Compare, bool Stat>
struct BinarySearchArray : public KeyValueArray<K, T, N, Stat> {
 protected:
  template <class K2, class T2, size_t N2, class Compare2, bool Stat2>
  friend struct BinarySearchArray;

  using MapStatisticsBase<Stat>::UpdateMaxCount;
  using MapStatisticsBase<Stat>::IncreaseInsertCount;
  using MapStatisticsBase<Stat>::IncreaseEraseCount;
  using MapStatisticsBase<Stat>::RecordFind;

  using KeyValueArray<K, T, N, Stat>::is_map;
  using KeyValueArray<K, T, N, Stat>::array_;
  using KeyValueArray<K, T, N, Stat>::_swap;

 public:
  using key_compare = Compare;
  using typename KeyValueArray<K, T, N, Stat>::value_type;
  using typename KeyValueArray<K, T, N, Stat>::store_value_type;
  using typename KeyValueArray<K, T, N, Stat>::iterator;
  using typename KeyValueArray<K, T, N, Stat>::const_iterator;
  using typename KeyValueArray<K, T, N, Stat>::store_iterator;
  using KeyValueArray<K, T, N, Stat>::begin;
  using KeyValueArray<K, T, N, Stat>::end;
  using KeyValueArray<K, T, N, Stat>::erase;

 public:
  BinarySearchArray() = default;
  BinarySearchArray(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  template <size_t N2>
  BinarySearchArray(const BinarySearchArray<K, T, N2, Compare, Stat>& other)
      : KeyValueArray<K, T, N, Stat>(other) {}

  template <size_t N2>
  BinarySearchArray(BinarySearchArray<K, T, N2, Compare, Stat>&& other)
      : KeyValueArray<K, T, N, Stat>(std::move(other)) {}

  template <size_t N2>
  BinarySearchArray& operator=(
      const BinarySearchArray<K, T, N2, Compare, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  BinarySearchArray& operator=(
      BinarySearchArray<K, T, N2, Compare, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    if constexpr (is_map) {
      auto pos = lower_bound(value.first);
      if (pos != end() && pos->first == value.first) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {
            reinterpret_cast<iterator>(array_.emplace(
                reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
                std::forward_as_tuple(value.first),
                std::forward_as_tuple(value.second))),
            true};
      }
    } else {
      auto pos = lower_bound(value);
      if (pos != end() && *pos == value) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {reinterpret_cast<iterator>(array_.emplace(
                    reinterpret_cast<store_iterator>(pos), value)),
                true};
      }
    }
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    if constexpr (is_map) {
      auto pos = lower_bound(value.first);
      if (pos != end() && pos->first == value.first) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {
            reinterpret_cast<iterator>(array_.emplace(
                reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
                std::forward_as_tuple(std::move(value.first)),
                std::forward_as_tuple(std::move(value.second)))),
            true};
      }
    } else {
      auto pos = lower_bound(value);
      if (pos != end() && *pos == value) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {reinterpret_cast<iterator>(array_.emplace(
                    reinterpret_cast<store_iterator>(pos), std::move(value))),
                true};
      }
    }
  }

  // Without this overload, `std::pair<K, T> v; map.insert(std::move(v));` would
  // implicitly construct `std::pair<const K, T>` from `std::move(v)`.
  template <class U, class = std::enable_if_t<
                         is_map && std::is_same_v<U, store_value_type>>>
  std::pair<iterator, bool> insert(U&& value) {
    auto pos = lower_bound(value.first);
    if (pos != end() && pos->first == value.first) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};  // duplicate
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(array_.emplace(
                  reinterpret_cast<store_iterator>(pos), std::move(value))),
              true};
    }
  }

  size_t erase(const K& key) {
    IncreaseEraseCount();
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      array_.erase(reinterpret_cast<store_iterator>(pos));
      return 1;
    } else {
      return 0;
    }
  }

  iterator find(const K& key) {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      return pos;
    } else {
      return end();
    }
  }

  const_iterator find(const K& key) const {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      return pos;
    } else {
      return end();
    }
  }

  bool contains(const K& key) const { return find(key) != end(); }

  size_t count(const K& key) const { return contains(key) ? 1 : 0; }

  template <size_t N2>
  void swap(BinarySearchArray<K, T, N2, Compare, Stat>& other) {
    _swap(other);
  }

  template <size_t N2>
  void merge(BinarySearchArray<K, T, N2, Compare, Stat>& other) {
    auto& other_array = other.array_;
    for (intptr_t i = other_array.size() - 1; i >= 0; i--) {
      if (insert(std::move(other_array[i])).second) {
        other_array.erase(other_array.begin() + i);
      }
    }
  }

  bool is_data_ordered() const { return true; }

  template <size_t N2>
  bool operator==(
      const BinarySearchArray<K, T, N2, Compare, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(
      const BinarySearchArray<K, T, N2, Compare, Stat>& other) const {
    return array_ != other.array_;
  }

 protected:
  BASE_VECTOR_NEVER_INLINE iterator lower_bound(const K& key) const {
    return const_cast<iterator>(std::lower_bound(
        begin(), end(), key, [](const value_type& item, const K& value) {
          return Compare()(*reinterpret_cast<const K*>(&item), value);
        }));
  }
};

/**
 * @brief Used to implement map/set, but only provides map/set style interface,
 * internal data is stored in array linearly, and search also uses linear
 * search.
 *
 * It is very suitable for scenarios that require a map/set interface, but the
 * amount of data is relatively small, and the number of container traversals is
 * far greater than the number of searches.
 */
template <class K, class T, size_t N, bool Stat>
struct LinearSearchArray : public KeyValueArray<K, T, N, Stat> {
 protected:
  template <class K2, class T2, size_t N2, bool Stat2>
  friend struct LinearSearchArray;

  using MapStatisticsBase<Stat>::UpdateMaxCount;
  using MapStatisticsBase<Stat>::IncreaseInsertCount;
  using MapStatisticsBase<Stat>::IncreaseEraseCount;
  using MapStatisticsBase<Stat>::RecordFind;

  using KeyValueArray<K, T, N, Stat>::is_map;
  using KeyValueArray<K, T, N, Stat>::array_;
  using KeyValueArray<K, T, N, Stat>::_swap;

 public:
  using key_compare = void;
  using typename KeyValueArray<K, T, N, Stat>::value_type;
  using typename KeyValueArray<K, T, N, Stat>::store_value_type;
  using typename KeyValueArray<K, T, N, Stat>::iterator;
  using typename KeyValueArray<K, T, N, Stat>::const_iterator;
  using typename KeyValueArray<K, T, N, Stat>::store_iterator;
  using KeyValueArray<K, T, N, Stat>::begin;
  using KeyValueArray<K, T, N, Stat>::end;
  using KeyValueArray<K, T, N, Stat>::erase;

 public:
  LinearSearchArray() = default;
  LinearSearchArray(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  LinearSearchArray(Vector<store_value_type>&& source_array)
      : KeyValueArray<K, T, N, Stat>(std::move(source_array)) {}

  LinearSearchArray& operator=(Vector<store_value_type>&& source_array) {
    array_ = std::move(source_array);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchArray(const LinearSearchArray<K, T, N2, Stat>& other)
      : KeyValueArray<K, T, N, Stat>(other) {}

  template <size_t N2>
  LinearSearchArray(LinearSearchArray<K, T, N2, Stat>&& other)
      : KeyValueArray<K, T, N, Stat>(std::move(other)) {}

  template <size_t N2>
  LinearSearchArray& operator=(const LinearSearchArray<K, T, N2, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchArray& operator=(LinearSearchArray<K, T, N2, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    if constexpr (is_map) {
      auto pos = find_exact(value.first);
      if (pos != nullptr) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {
            reinterpret_cast<iterator>(&array_.emplace_back(
                std::piecewise_construct, std::forward_as_tuple(value.first),
                std::forward_as_tuple(value.second))),
            true};
      }
    } else {
      auto pos = find_exact(value);
      if (pos != nullptr) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {reinterpret_cast<iterator>(&array_.emplace_back(value)), true};
      }
    }
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    if constexpr (is_map) {
      auto pos = find_exact(value.first);
      if (pos != nullptr) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {reinterpret_cast<iterator>(&array_.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(value.first)),
                    std::forward_as_tuple(std::move(value.second)))),
                true};
      }
    } else {
      auto pos = find_exact(value);
      if (pos != nullptr) {
        RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
        return {pos, false};  // duplicate
      } else {
        RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
        return {
            reinterpret_cast<iterator>(&array_.emplace_back(std::move(value))),
            true};
      }
    }
  }

  // Without this overload, `std::pair<K, T> v; map.insert(std::move(v));` would
  // implicitly construct `std::pair<const K, T>` from `std::move(v)`.
  template <class U, class = std::enable_if_t<
                         is_map && std::is_same_v<U, store_value_type>>>
  std::pair<iterator, bool> insert(U&& value) {
    auto pos = find_exact(value.first);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};  // duplicate
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(&array_.emplace_back(std::move(value))),
          true};
    }
  }

  size_t erase(const K& key) {
    IncreaseEraseCount();
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = find_exact(key);
    if (pos != nullptr) {
      array_.erase(reinterpret_cast<store_iterator>(pos));
      return 1;
    } else {
      return 0;
    }
  }

  iterator find(const K& key) {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = find_exact(key);
    if (pos != nullptr) {
      return pos;
    } else {
      return end();
    }
  }

  const_iterator find(const K& key) const {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = find_exact(key);
    if (pos != nullptr) {
      return pos;
    } else {
      return end();
    }
  }

  bool contains(const K& key) const {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    return find_exact(key) != nullptr;
  }

  size_t count(const K& key) const { return contains(key) ? 1 : 0; }

  template <size_t N2>
  void swap(LinearSearchArray<K, T, N2, Stat>& other) {
    _swap(other);
  }

  template <size_t N2>
  void merge(LinearSearchArray<K, T, N2, Stat>& other) {
    auto& other_array = other.array_;
    for (intptr_t i = other_array.size() - 1; i >= 0; i--) {
      if (insert(std::move(other_array[i])).second) {
        other_array.erase(other_array.begin() + i);
      }
    }
  }

  bool is_data_ordered() const { return false; }

  template <size_t N2>
  bool operator==(const LinearSearchArray<K, T, N2, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const LinearSearchArray<K, T, N2, Stat>& other) const {
    return array_ != other.array_;
  }

 protected:
  BASE_VECTOR_NEVER_INLINE iterator find_exact(const K& key) const {
    for (auto it = array_.begin(), end = array_.end(); it != end; it++) {
      if (*reinterpret_cast<const K*>(it) == key) {
        return reinterpret_cast<iterator>(const_cast<store_iterator>(it));
      }
    }
    return nullptr;
  }
};

template <class K, class T, size_t N, class Compare, bool Stat>
struct BinarySearchMap : public BinarySearchArray<K, T, N, Compare, Stat> {
 protected:
  template <class K2, class T2, size_t N2, class Compare2, bool Stat2>
  friend struct BinarySearchMap;

  using MapStatisticsBase<Stat>::UpdateMaxCount;
  using MapStatisticsBase<Stat>::IncreaseInsertCount;
  using MapStatisticsBase<Stat>::IncreaseEraseCount;
  using MapStatisticsBase<Stat>::RecordFind;

  using KeyValueArray<K, T, N, Stat>::array_;

 public:
  using typename BinarySearchArray<K, T, N, Compare, Stat>::value_type;
  using typename BinarySearchArray<K, T, N, Compare, Stat>::iterator;
  using typename BinarySearchArray<K, T, N, Compare, Stat>::store_iterator;
  using BinarySearchArray<K, T, N, Compare, Stat>::insert;
  using BinarySearchArray<K, T, N, Compare, Stat>::lower_bound;
  using KeyValueArray<K, T, N, Stat>::end;

  BinarySearchMap() = default;
  BinarySearchMap(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  template <size_t N2>
  BinarySearchMap(const BinarySearchMap<K, T, N2, Compare, Stat>& other)
      : BinarySearchArray<K, T, N, Compare, Stat>(other) {}

  template <size_t N2>
  BinarySearchMap(BinarySearchMap<K, T, N2, Compare, Stat>&& other)
      : BinarySearchArray<K, T, N, Compare, Stat>(std::move(other)) {}

  template <size_t N2>
  BinarySearchMap& operator=(
      const BinarySearchMap<K, T, N2, Compare, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  BinarySearchMap& operator=(BinarySearchMap<K, T, N2, Compare, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <class U>
  T& operator[](U&& key) {
    return try_insert(std::forward<U>(key))->second;
  }

  T& at(const K& key) {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      return pos->second;
    }
    ::abort();  // Always nothrow
  }

  const T& at(const K& key) const {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      return pos->second;
    }
    ::abort();  // Always nothrow
  }

  template <class V>
  std::pair<iterator, bool> insert_or_assign(const K& key, V&& value) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      pos->second = std::forward<V>(value);
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(array_.emplace(
                  reinterpret_cast<store_iterator>(pos),
                  std::piecewise_construct, std::forward_as_tuple(key),
                  std::forward_as_tuple(std::forward<V>(value)))),
              true};
    }
  }

  template <class V>
  std::pair<iterator, bool> insert_or_assign(K&& key, V&& value) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      pos->second = std::forward<V>(value);
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(array_.emplace(
              reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
              std::forward_as_tuple(std::move(key)),
              std::forward_as_tuple(std::forward<V>(value)))),
          true};
    }
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(const K& key, Args&&... args) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(array_.emplace(
                  reinterpret_cast<store_iterator>(pos),
                  std::piecewise_construct, std::forward_as_tuple(key),
                  std::forward_as_tuple(std::forward<Args>(args)...))),
              true};
    }
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(K&& key, Args&&... args) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(array_.emplace(
              reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
              std::forward_as_tuple(std::move(key)),
              std::forward_as_tuple(std::forward<Args>(args)...))),
          true};
    }
  }

  template <class U, class... Args>
  std::pair<iterator, bool> try_emplace(U&& key, Args&&... args) {
    return emplace(std::forward<U>(key), std::forward<Args>(args)...);
  }

  template <typename... Args1, typename... Args2>
  std::pair<iterator, bool> emplace(std::piecewise_construct_t,
                                    std::tuple<Args1...> args1,
                                    std::tuple<Args2...> args2) {
    // Construct key in first place.
    K key = std::make_from_tuple<K>(std::move(args1));

    // Do emplace.
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};  // key is discarded
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(array_.emplace(
              reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
              std::forward_as_tuple(std::move(key)), std::move(args2))),
          true};
    }
  }

  template <size_t N2>
  bool operator==(const BinarySearchMap<K, T, N2, Compare, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const BinarySearchMap<K, T, N2, Compare, Stat>& other) const {
    return array_ != other.array_;
  }

 protected:
  iterator try_insert(const K& key) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return pos;
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return reinterpret_cast<iterator>(array_.emplace(
          reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
          std::forward_as_tuple(key), std::tuple<>()));
    }
  }

  iterator try_insert(K&& key) {
    auto pos = lower_bound(key);
    if (pos != end() && *reinterpret_cast<const K*>(pos) == key) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return pos;
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return reinterpret_cast<iterator>(array_.emplace(
          reinterpret_cast<store_iterator>(pos), std::piecewise_construct,
          std::forward_as_tuple(std::move(key)), std::tuple<>()));
    }
  }
};

template <class K, size_t N, class Compare, bool Stat>
struct BinarySearchSet : public BinarySearchArray<K, void, N, Compare, Stat> {
 protected:
  template <class K2, size_t N2, class Compare2, bool Stat2>
  friend struct BinarySearchSet;

  using MapStatisticsBase<Stat>::UpdateMaxCount;

  using BinarySearchArray<K, void, N, Compare, Stat>::array_;

 public:
  using typename BinarySearchArray<K, void, N, Compare, Stat>::value_type;
  using typename BinarySearchArray<K, void, N, Compare, Stat>::iterator;
  using BinarySearchArray<K, void, N, Compare, Stat>::insert;

  BinarySearchSet() = default;
  BinarySearchSet(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  template <size_t N2>
  BinarySearchSet(const BinarySearchSet<K, N2, Compare, Stat>& other)
      : BinarySearchArray<K, void, N, Compare, Stat>(other) {}

  template <size_t N2>
  BinarySearchSet(BinarySearchSet<K, N2, Compare, Stat>&& other)
      : BinarySearchArray<K, void, N, Compare, Stat>(std::move(other)) {}

  template <size_t N2>
  BinarySearchSet& operator=(
      const BinarySearchSet<K, N2, Compare, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  BinarySearchSet& operator=(BinarySearchSet<K, N2, Compare, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(K(std::forward<Args>(args)...));
  }

  template <size_t N2>
  bool operator==(const BinarySearchSet<K, N2, Compare, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const BinarySearchSet<K, N2, Compare, Stat>& other) const {
    return array_ != other.array_;
  }
};

template <class K, class T, size_t N, bool Stat>
struct LinearSearchMap : public LinearSearchArray<K, T, N, Stat> {
 protected:
  template <class K2, class T2, size_t N2, bool Stat2>
  friend struct LinearSearchMap;

  using MapStatisticsBase<Stat>::UpdateMaxCount;
  using MapStatisticsBase<Stat>::IncreaseInsertCount;
  using MapStatisticsBase<Stat>::IncreaseEraseCount;
  using MapStatisticsBase<Stat>::RecordFind;

  using KeyValueArray<K, T, N, Stat>::array_;

 public:
  using typename LinearSearchArray<K, T, N, Stat>::value_type;
  using typename LinearSearchArray<K, T, N, Stat>::store_value_type;
  using typename LinearSearchArray<K, T, N, Stat>::iterator;
  using typename LinearSearchArray<K, T, N, Stat>::store_iterator;
  using LinearSearchArray<K, T, N, Stat>::insert;
  using LinearSearchArray<K, T, N, Stat>::find_exact;
  using KeyValueArray<K, T, N, Stat>::end;

  LinearSearchMap() = default;
  LinearSearchMap(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  LinearSearchMap(Vector<store_value_type>&& source_array)
      : LinearSearchArray<K, T, N, Stat>(std::move(source_array)) {}

  LinearSearchMap& operator=(Vector<store_value_type>&& source_array) {
    array_ = std::move(source_array);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchMap(const LinearSearchMap<K, T, N2, Stat>& other)
      : LinearSearchArray<K, T, N, Stat>(other) {}

  template <size_t N2>
  LinearSearchMap(LinearSearchMap<K, T, N2, Stat>&& other)
      : LinearSearchArray<K, T, N, Stat>(std::move(other)) {}

  template <size_t N2>
  LinearSearchMap& operator=(const LinearSearchMap<K, T, N2, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchMap& operator=(LinearSearchMap<K, T, N2, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <class U>
  T& operator[](U&& key) {
    return try_insert(std::forward<U>(key))->second;
  }

  T& at(const K& key) {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = find_exact(key);
    if (pos != nullptr) {
      return pos->second;
    }
    ::abort();  // Always nothrow
  }

  const T& at(const K& key) const {
    RecordFind(MapStatisticsFindKind::kFind, array_.size());
    auto pos = find_exact(key);
    if (pos != nullptr) {
      return pos->second;
    }
    ::abort();  // Always nothrow
  }

  template <class V>
  std::pair<iterator, bool> insert_or_assign(const K& key, V&& value) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      pos->second = std::forward<V>(value);
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(&array_.emplace_back(
                  std::piecewise_construct, std::forward_as_tuple(key),
                  std::forward_as_tuple(std::forward<V>(value)))),
              true};
    }
  }

  template <class V>
  std::pair<iterator, bool> insert_or_assign(K&& key, V&& value) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      pos->second = std::forward<V>(value);
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(&array_.emplace_back(
              std::piecewise_construct, std::forward_as_tuple(std::move(key)),
              std::forward_as_tuple(std::forward<V>(value)))),
          true};
    }
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(const K& key, Args&&... args) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(&array_.emplace_back(
                  std::piecewise_construct, std::forward_as_tuple(key),
                  std::forward_as_tuple(std::forward<Args>(args)...))),
              true};
    }
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(K&& key, Args&&... args) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {
          reinterpret_cast<iterator>(&array_.emplace_back(
              std::piecewise_construct, std::forward_as_tuple(std::move(key)),
              std::forward_as_tuple(std::forward<Args>(args)...))),
          true};
    }
  }

  template <class U, class... Args>
  std::pair<iterator, bool> try_emplace(U&& key, Args&&... args) {
    return emplace(std::forward<U>(key), std::forward<Args>(args)...);
  }

  template <typename... Args1, typename... Args2>
  std::pair<iterator, bool> emplace(std::piecewise_construct_t,
                                    std::tuple<Args1...> args1,
                                    std::tuple<Args2...> args2) {
    // Construct key in first place.
    K key = std::make_from_tuple<K>(std::move(args1));

    // Do emplace.
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return {pos, false};  // key is discarded
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return {reinterpret_cast<iterator>(&array_.emplace_back(
                  std::piecewise_construct,
                  std::forward_as_tuple(std::move(key)), std::move(args2))),
              true};
    }
  }

  template <size_t N2>
  bool operator==(const LinearSearchMap<K, T, N2, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const LinearSearchMap<K, T, N2, Stat>& other) const {
    return array_ != other.array_;
  }

 protected:
  iterator try_insert(const K& key) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return pos;
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return reinterpret_cast<iterator>(
          &array_.emplace_back(std::piecewise_construct,
                               std::forward_as_tuple(key), std::tuple<>()));
    }
  }

  iterator try_insert(K&& key) {
    auto pos = find_exact(key);
    if (pos != nullptr) {
      RecordFind(MapStatisticsFindKind::kInsertFindCollision, array_.size());
      return pos;
    } else {
      RecordFind(MapStatisticsFindKind::kInsertFind, array_.size());
      return reinterpret_cast<iterator>(&array_.emplace_back(
          std::piecewise_construct, std::forward_as_tuple(std::move(key)),
          std::tuple<>()));
    }
  }
};

template <class K, size_t N, bool Stat>
struct LinearSearchSet : public LinearSearchArray<K, void, N, Stat> {
 protected:
  template <class K2, size_t N2, bool Stat2>
  friend struct LinearSearchSet;

  using MapStatisticsBase<Stat>::UpdateMaxCount;

  using LinearSearchArray<K, void, N, Stat>::array_;

 public:
  using typename LinearSearchArray<K, void, N, Stat>::value_type;
  using typename LinearSearchArray<K, void, N, Stat>::store_value_type;
  using typename LinearSearchArray<K, void, N, Stat>::iterator;
  using LinearSearchArray<K, void, N, Stat>::insert;

  LinearSearchSet() = default;
  LinearSearchSet(std::initializer_list<value_type> list) {
    array_.reserve(list.size());
    for (auto& v : list) {
      insert(std::move(v));
    }
  }

  LinearSearchSet(Vector<store_value_type>&& source_array)
      : LinearSearchArray<K, void, N, Stat>(std::move(source_array)) {}

  LinearSearchSet& operator=(Vector<store_value_type>&& source_array) {
    array_ = std::move(source_array);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchSet(const LinearSearchSet<K, N2, Stat>& other)
      : LinearSearchArray<K, void, N, Stat>(other) {}

  template <size_t N2>
  LinearSearchSet(LinearSearchSet<K, N2, Stat>&& other)
      : LinearSearchArray<K, void, N, Stat>(std::move(other)) {}

  template <size_t N2>
  LinearSearchSet& operator=(const LinearSearchSet<K, N2, Stat>& other) {
    array_ = other.array_;
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <size_t N2>
  LinearSearchSet& operator=(LinearSearchSet<K, N2, Stat>&& other) {
    array_ = std::move(other.array_);
    UpdateMaxCount(array_.size());
    return *this;
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(K(std::forward<Args>(args)...));
  }

  template <size_t N2>
  bool operator==(const LinearSearchSet<K, N2, Stat>& other) const {
    return array_ == other.array_;
  }

  template <size_t N2>
  bool operator!=(const LinearSearchSet<K, N2, Stat>& other) const {
    return array_ != other.array_;
  }
};

template <class K, class T, class Compare = std::less<K>>
using OrderedFlatMap = BinarySearchMap<K, T, 0, Compare, false>;

template <class K, class T, size_t N, class Compare = std::less<K>>
using InlineOrderedFlatMap = BinarySearchMap<K, T, N, Compare, false>;

template <class K, class Compare = std::less<K>>
using OrderedFlatSet = BinarySearchSet<K, 0, Compare, false>;

template <class K, size_t N, class Compare = std::less<K>>
using InlineOrderedFlatSet = BinarySearchSet<K, N, Compare, false>;

template <class K, class T>
using LinearFlatMap = LinearSearchMap<K, T, 0, false>;

template <class K, class T, size_t N>
using InlineLinearFlatMap = LinearSearchMap<K, T, N, false>;

template <class K>
using LinearFlatSet = LinearSearchSet<K, 0, false>;

template <class K, size_t N>
using InlineLinearFlatSet = LinearSearchSet<K, N, false>;

/**
 Helper to declare inlined version of existing map type.
    using MapType = OrderedFlatMap<int, int>;
    Inlined<MapType, 5>::type inlined_map;
 */
template <class From, size_t N>
struct Inlined {
  using type = std::conditional_t<
      std::is_same_v<From, OrderedFlatMap<typename From::key_type,
                                          typename From::mapped_type,
                                          typename From::key_compare>>,
      InlineOrderedFlatMap<typename From::key_type, typename From::mapped_type,
                           N, typename From::key_compare>,
      std::conditional_t<
          std::is_same_v<From, OrderedFlatSet<typename From::key_type,
                                              typename From::key_compare>>,
          InlineOrderedFlatSet<typename From::key_type, N,
                               typename From::key_compare>,

          std::conditional_t<
              std::is_same_v<From, LinearFlatMap<typename From::key_type,
                                                 typename From::mapped_type>>,
              InlineLinearFlatMap<typename From::key_type,
                                  typename From::mapped_type, N>,

              std::conditional_t<
                  std::is_same_v<From, LinearFlatSet<typename From::key_type>>,
                  InlineLinearFlatSet<typename From::key_type, N>, void>>>>;
};

static_assert(std::is_same_v<Inlined<OrderedFlatMap<int, int>, 5>::type,
                             InlineOrderedFlatMap<int, int, 5>>);
static_assert(std::is_same_v<Inlined<LinearFlatMap<int, int>, 5>::type,
                             InlineLinearFlatMap<int, int, 5>>);

static_assert(std::is_same_v<Inlined<OrderedFlatSet<int>, 5>::type,
                             InlineOrderedFlatSet<int, 5>>);
static_assert(std::is_same_v<Inlined<LinearFlatSet<int>, 5>::type,
                             InlineLinearFlatSet<int, 5>>);

static_assert(sizeof(OrderedFlatMap<int, int>) ==
              sizeof(Vector<std::pair<int, int>>));

static_assert(sizeof(OrderedFlatSet<int>) == sizeof(Vector<int>));

static_assert(sizeof(LinearFlatMap<int, int>) ==
              sizeof(Vector<std::pair<int, int>>));

static_assert(sizeof(LinearFlatSet<int>) == sizeof(Vector<int>));

}  // namespace base
}  // namespace lynx

namespace std {
template <class T>
inline void swap(lynx::base::Vector<T>& x, lynx::base::Vector<T>& y) {
  x.swap(y);
}

template <class K, class T, size_t N1, size_t N2, class Compare1,
          class Compare2, bool Stat1, bool Stat2,
          typename = typename std::enable_if<N1 != N2>::type>
inline void swap(lynx::base::BinarySearchMap<K, T, N1, Compare1, Stat1>& x,
                 lynx::base::BinarySearchMap<K, T, N2, Compare2, Stat2>& y) {
  x.swap(y);
}

template <class K, size_t N1, size_t N2, class Compare1, class Compare2,
          bool Stat1, bool Stat2,
          typename = typename std::enable_if<N1 != N2>::type>
inline void swap(lynx::base::BinarySearchSet<K, N1, Compare1, Stat1>& x,
                 lynx::base::BinarySearchSet<K, N2, Compare2, Stat2>& y) {
  x.swap(y);
}

template <class K, class T, size_t N1, size_t N2, bool Stat1, bool Stat2,
          typename = typename std::enable_if<N1 != N2>::type>
inline void swap(lynx::base::LinearSearchMap<K, T, N1, Stat1>& x,
                 lynx::base::LinearSearchMap<K, T, N2, Stat2>& y) {
  x.swap(y);
}

template <class K, size_t N1, size_t N2, bool Stat1, bool Stat2,
          typename = typename std::enable_if<N1 != N2>::type>
inline void swap(lynx::base::LinearSearchSet<K, N1, Stat1>& x,
                 lynx::base::LinearSearchSet<K, N2, Stat2>& y) {
  x.swap(y);
}
}  // namespace std

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-template"

#endif  // BASE_INCLUDE_VECTOR_H_
