// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_SHARED_DATA_WHITE_BOARD_INSPECTOR_H_
#define CORE_SHARED_DATA_WHITE_BOARD_INSPECTOR_H_

#include <memory>
#include <string>

#include "core/public/pub_value.h"

namespace lynx {
namespace tasm {

class WhiteBoard;

class WhiteBoardInspector {
 public:
  WhiteBoardInspector() = default;
  virtual ~WhiteBoardInspector() = default;

  void SetWhiteBoard(const std::shared_ptr<WhiteBoard>& white_board) {
    white_board_ = white_board;
  }

  virtual void OnSharedDataAdded(const std::string& key,
                                 const pub::Value& value) = 0;
  virtual void OnSharedDataUpdated(const std::string& key,
                                   const pub::Value& value) = 0;
  virtual void OnSharedDataRemoved(const std::string& key) = 0;
  virtual void OnSharedDataCleared() = 0;

 protected:
  std::weak_ptr<WhiteBoard> white_board_;
};

}  // namespace tasm
}  // namespace lynx

#endif  // CORE_SHARED_DATA_WHITE_BOARD_INSPECTOR_H_
