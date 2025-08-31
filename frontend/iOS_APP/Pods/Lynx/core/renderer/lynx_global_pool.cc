// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "core/renderer/lynx_global_pool.h"

#include "core/renderer/utils/lynx_env.h"

namespace lynx {
namespace tasm {

LynxGlobalPool& LynxGlobalPool::GetInstance() {
  static base::NoDestructor<LynxGlobalPool> instance;
  return *instance;
}

void LynxGlobalPool::PreparePool() {
  constexpr int32_t kGlobalQuickContextPoolSize = 5;
  int32_t size = tasm::LynxEnv::GetInstance().GetGlobalQuickContextPoolSize(
      kGlobalQuickContextPoolSize);
  quick_context_pool_->FillPool(size);
}

}  // namespace tasm
}  // namespace lynx
