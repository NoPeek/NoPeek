// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "lynx_layout_proxy.h"

#include <memory>
#include <utility>

namespace lynx {
namespace shell {

void LynxLayoutProxy::DispatchTaskToLynxLayout(base::closure task) {
  actor_->Act([task = std::move(task)](auto& layout) mutable { task(); });
}
}  // namespace shell
}  // namespace lynx
