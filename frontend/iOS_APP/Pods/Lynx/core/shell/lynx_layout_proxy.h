// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_SHELL_LYNX_LAYOUT_PROXY_H_
#define CORE_SHELL_LYNX_LAYOUT_PROXY_H_

#include <memory>

#include "base/include/closure.h"
#include "base/include/lynx_actor.h"
#include "core/renderer/ui_wrapper/layout/layout_context.h"
#include "lynx_engine.h"

namespace lynx {
namespace shell {
class LynxLayoutProxy {
 public:
  explicit LynxLayoutProxy(
      const std::shared_ptr<shell::LynxActor<tasm::LayoutContext>>& actor)
      : actor_(actor) {}
  virtual ~LynxLayoutProxy() = default;

  LynxLayoutProxy(const LynxLayoutProxy&) = delete;
  LynxLayoutProxy& operator=(const LynxLayoutProxy&) = delete;

  void DispatchTaskToLynxLayout(base::closure task);

 private:
  std::shared_ptr<shell::LynxActor<tasm::LayoutContext>> actor_;
};

}  // namespace shell
}  // namespace lynx

#endif  // CORE_SHELL_LYNX_LAYOUT_PROXY_H_
