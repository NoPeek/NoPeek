// Copyright 2020 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef CORE_STYLE_TIMING_FUNCTION_DATA_H_
#define CORE_STYLE_TIMING_FUNCTION_DATA_H_

#include <tuple>

#include "core/renderer/starlight/style/css_type.h"

namespace lynx {
namespace starlight {
struct TimingFunctionData {
  static constexpr int INDEX_TYPE = 0;
  static constexpr int INDEX_X1 = 1;
  static constexpr int INDEX_Y1 = 2;
  static constexpr int INDEX_X2 = 3;
  static constexpr int INDEX_Y2 = 4;
  static constexpr int INDEX_STEPS_TYPE = 2;

  float x1{0.0f};
  float y1{0.0f};
  float x2{0.0f};
  float y2{0.0f};
  starlight::TimingFunctionType timing_func{TimingFunctionType::kLinear};
  starlight::StepsType steps_type{StepsType::kInvalid};
  TimingFunctionData() = default;
  ~TimingFunctionData() = default;
  void Reset();
  bool operator==(const TimingFunctionData& rhs) const {
    return std::tie(timing_func, x1, y1, x2, y2, steps_type) ==
           std::tie(rhs.timing_func, rhs.x1, rhs.y1, rhs.x2, rhs.y2,
                    rhs.steps_type);
  }
};
}  // namespace starlight
}  // namespace lynx

#endif  // CORE_STYLE_TIMING_FUNCTION_DATA_H_
