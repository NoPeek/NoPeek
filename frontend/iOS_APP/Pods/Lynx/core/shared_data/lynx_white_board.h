// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_SHARED_DATA_LYNX_WHITE_BOARD_H_
#define CORE_SHARED_DATA_LYNX_WHITE_BOARD_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/include/base_export.h"
#include "base/include/closure.h"
#include "base/include/fml/synchronization/shared_mutex.h"
#include "core/public/pub_value.h"
#include "core/shared_data/white_board_inspector.h"

namespace lynx {
namespace tasm {

/**
 WhiteBoard is a DataCenter that can be shared and operated by multi LynxViews,
 it is not thread-safe, should be operated only on TASM thread Now. users can
 `set` `get` `registerListener` to whiteboard for sharing data between multiple
 lynxViews.
 */
enum class WhiteBoardStorageType : uint8_t {
  TYPE_LEPUS = 0,
  TYPE_JS,
  TYPE_CLIENT,

  // ADDED_BEBORE!!
  COUNT
};

struct WhiteBoardListener {
  double callback_id;
  // invoked while new data is received.
  base::MoveOnlyClosure<void, const pub::Value&> trigger_callback;
  // invoked while being removed.
  base::MoveOnlyClosure<void> remove_callback;
};

class WhiteBoardDelegate;
class WhiteBoard final {
 public:
  WhiteBoard();
  WhiteBoard(const WhiteBoard&) = delete;
  WhiteBoard& operator=(const WhiteBoard&) = delete;
  WhiteBoard(WhiteBoard&&) = delete;
  WhiteBoard& operator=(WhiteBoard&&) = delete;

  std::shared_ptr<WhiteBoardInspector> GetInspector() { return inspector_; }
  void SetInspector(const std::shared_ptr<WhiteBoardInspector>& inspector) {
    inspector_ = inspector;
  }

  // set & get operation
  BASE_EXPORT_FOR_DEVTOOL void SetGlobalSharedData(
      const std::string& key, const std::shared_ptr<pub::Value>& value);
  BASE_EXPORT_FOR_DEVTOOL std::shared_ptr<pub::Value> GetGlobalSharedData(
      const std::string& key);

  BASE_EXPORT_FOR_DEVTOOL void RemoveGlobalSharedData(const std::string& key);
  BASE_EXPORT_FOR_DEVTOOL void ClearGlobalSharedData();

  // subscribe & unsubscribe operation
  void RegisterSharedDataListener(const WhiteBoardStorageType& type,
                                  const std::string& key,
                                  WhiteBoardListener listener);
  void RemoveSharedDataListener(const WhiteBoardStorageType& type,
                                const std::string& key, int32_t listener_id);

  ~WhiteBoard() = default;

  // only used for inspector
  BASE_EXPORT_FOR_DEVTOOL const auto& GetAllGlobalSharedData() {
    return data_center_;
  }

 private:
  using LynxWhiteBoardMap =
      std::unordered_map<std::string, std::shared_ptr<pub::Value>>;
  using WhiteBoardListenerMap =
      std::unordered_map<std::string, std::vector<WhiteBoardListener>>;

  void TriggerListener(const WhiteBoardStorageType& type,
                       const std::string& key, const pub::Value& value);

  LynxWhiteBoardMap data_center_;
  std::unique_ptr<fml::SharedMutex> data_center_lock_;
  std::unordered_map<WhiteBoardStorageType, std::unique_ptr<fml::SharedMutex>>
      listener_lock_;
  std::vector<WhiteBoardListenerMap> listeners_;
  std::shared_ptr<WhiteBoardInspector> inspector_;
};

}  // namespace tasm
}  // namespace lynx

#endif  // CORE_SHARED_DATA_LYNX_WHITE_BOARD_H_
