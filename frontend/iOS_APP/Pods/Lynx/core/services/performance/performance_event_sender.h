// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_SERVICES_PERFORMANCE_PERFORMANCE_EVENT_SENDER_H_
#define CORE_SERVICES_PERFORMANCE_PERFORMANCE_EVENT_SENDER_H_

#include <cstddef>
#include <memory>
#include <utility>

#include "core/public/pub_value.h"

namespace lynx {
namespace tasm {
namespace performance {

using EventType = uint8_t;
inline constexpr EventType kEventTypePlatform = 1 << 0;
inline constexpr EventType kEventTypeBTSEngine = 1 << 1;
inline constexpr EventType kEventTypeMTSEngine = 1 << 2;
inline constexpr EventType kEventTypeAll =
    kEventTypePlatform | kEventTypeBTSEngine | kEventTypeMTSEngine;

inline constexpr char kPerformanceEventType[] = "entryType";
inline constexpr char kPerformanceEventName[] = "name";

class PerformanceEventSender {
 public:
  explicit PerformanceEventSender(
      std::shared_ptr<pub::PubValueFactory> value_factory)
      : value_factory_(std::move(value_factory)) {}
  virtual ~PerformanceEventSender() = default;
  // Called when a performance event occurs.
  // This is a pure virtual function and must be implemented by derived classes.
  // @param entry A unique pointer to a pub::Value object containing the
  //              performance entry data.
  // @param env The environment(s) to which this performance event pertains.
  //            Defaults to all environments.
  virtual void OnPerformanceEvent(std::unique_ptr<lynx::pub::Value> entry,
                                  EventType type = kEventTypeAll) = 0;
  virtual const std::shared_ptr<pub::PubValueFactory>& GetValueFactory() {
    return value_factory_;
  };

  virtual void SetEnableMainThreadCallback(bool enable) {
    enable_main_thread_engine_callback_ = enable;
  };

  virtual bool GetEnableMainThreadCallback() const {
    return enable_main_thread_engine_callback_;
  };

 protected:
  std::shared_ptr<pub::PubValueFactory> value_factory_;
  bool enable_main_thread_engine_callback_{false};
};

}  // namespace performance
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_SERVICES_PERFORMANCE_PERFORMANCE_EVENT_SENDER_H_
