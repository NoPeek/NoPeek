// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_PUBLIC_PERFORMANCE_CONTROLLER_PLATFORM_IMPL_H_
#define CORE_PUBLIC_PERFORMANCE_CONTROLLER_PLATFORM_IMPL_H_

#include <memory>

#include "base/include/lynx_actor.h"
#include "core/public/pub_value.h"

namespace lynx {
namespace tasm {
namespace performance {
class PerformanceController;
class PerformanceControllerPlatformImpl {
 public:
  virtual ~PerformanceControllerPlatformImpl() = default;
  virtual void SetActor(
      const std::shared_ptr<shell::LynxActor<PerformanceController>>&
          actor) = 0;
  virtual void OnPerformanceEvent(const std::unique_ptr<pub::Value>& entry) = 0;
};
}  // namespace performance
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_PUBLIC_PERFORMANCE_CONTROLLER_PLATFORM_IMPL_H_
