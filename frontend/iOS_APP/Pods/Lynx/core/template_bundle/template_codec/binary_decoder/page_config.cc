// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "core/template_bundle/template_codec/binary_decoder/page_config.h"

#include <vector>

namespace lynx {
namespace tasm {

/**
 * @name: pipelineSchedulerConfig
 * @description: Scheduler config for pipeline, including
 * enableParallelElement/list-framework batch render and other scheduler config
 * @platform: Both
 * @supportVersion: 3.1
 */
static constexpr const char* const kPipelineSchedulerConfig =
    "pipelineSchedulerConfig";

/**
 * @name: enableNativeList
 * @description: Indicates whether use c++ list.
 * @supportVersion: 3.2
 */
static constexpr const char* const kEnableNativeList = "enableNativeList";

const PageConfig::PageConfigMap<TernaryBool>& PageConfig::GetFuncBoolMap() {
  static const base::NoDestructor<const PageConfigMap<TernaryBool>>
      kPageConfigFuncBoolMap{{
          {"trailNewImage",
           {&PageConfig::SetTrailNewImage, &PageConfig::GetTrailNewImage}},
          {"asyncRedirect",
           {&PageConfig::SetAsyncRedirectUrl,
            &PageConfig::GetAsyncRedirectUrl}},
          {"enableUseMapBuffer",
           {&PageConfig::SetEnableUseMapBuffer,
            &PageConfig::GetEnableUseMapBuffer}},
          {"enableUIOperationOptimize",
           {&PageConfig::SetEnableUIOperationOptimize,
            &PageConfig::GetEnableUIOperationOptimize}},
          {kEnableNativeList,
           {&PageConfig::SetEnableNativeList,
            &PageConfig::GetEnableNativeList}},
          {"enableFiberElementForRadonDiff",
           {&PageConfig::SetEnableFiberElementForRadonDiff,
            &PageConfig::GetEnableFiberElementForRadonDiff}},
          {"enableMicrotaskPromisePolyfill",
           {&PageConfig::SetEnableMicrotaskPromisePolyfill,
            &PageConfig::GetEnableMicrotaskPromisePolyfill}},
          {kEnableSignalAPI,
           {&PageConfig::SetEnableSignalAPI, &PageConfig::GetEnableSignalAPI}},
          {"enableOptPushStyleToBundle",
           {&PageConfig::SetEnableOptPushStyleToBundle,
            &PageConfig::GetEnableOptPushStyleToBundle}},
          {kEnableNativeScheduleCreateViewAsync,
           {&PageConfig::SetEnableNativeScheduleCreateViewAsync,
            &PageConfig::GetEnableNativeScheduleCreateViewAsync}},
          {"enableUnifiedPipeline",
           {&PageConfig::SetEnableUnifiedPipeline,
            &PageConfig::GetEnableUnifiedPipeline}},
      }};
  return *kPageConfigFuncBoolMap;
}

const PageConfig::PageConfigMap<uint64_t>& PageConfig::GetFuncUint64Map() {
  static const base::NoDestructor<const PageConfigMap<uint64_t>>
      kPageConfigFuncUint64Map{{{kPipelineSchedulerConfig,
                                 {&PageConfig::SetPipelineSchedulerConfig,
                                  &PageConfig::GetPipelineSchedulerConfig}}}};
  return *kPageConfigFuncUint64Map;
}

bool PageConfig::GetEnableParallelElement() const {
  bool enableParallelElementFromSchedulerConfig =
      (GetPipelineSchedulerConfig() & kEnableParallelElementMask) > 0;
  bool isParallelElementConfigUndefined =
      ((GetPipelineSchedulerConfig() & kDisableParallelElementMask) == 0) &&
      !enableParallelElementFromSchedulerConfig;
  // enableParallelElement from pipelineSchedulerConfig would override
  // enableParallelElement encode option
  return (enableParallelElementFromSchedulerConfig ||
          (isParallelElementConfigUndefined && enable_parallel_element_));
}

}  // namespace tasm
}  // namespace lynx
