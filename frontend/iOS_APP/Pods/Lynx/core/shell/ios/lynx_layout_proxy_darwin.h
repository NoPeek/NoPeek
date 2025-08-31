// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_SHELL_IOS_LYNX_LAYOUT_PROXY_DARWIN_H_
#define CORE_SHELL_IOS_LYNX_LAYOUT_PROXY_DARWIN_H_

#import <Foundation/Foundation.h>

#include <memory>

#include "core/shell/lynx_layout_proxy.h"

namespace lynx {
namespace shell {

class LynxLayoutProxyDarwin {
 public:
  explicit LynxLayoutProxyDarwin(
      std::shared_ptr<shell::LynxActor<tasm::LayoutContext>> actor)
      : layout_proxy_(std::make_unique<LynxLayoutProxy>(actor)) {}
  ~LynxLayoutProxyDarwin() = default;
  void RunOnLayoutThread(dispatch_block_t task);

 private:
  std::unique_ptr<LynxLayoutProxy> layout_proxy_;
};

}  // namespace shell
}  // namespace lynx

#endif  // CORE_SHELL_IOS_LYNX_LAYOUT_PROXY_DARWIN_H_
