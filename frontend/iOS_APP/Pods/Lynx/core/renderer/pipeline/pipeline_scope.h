// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_RENDERER_PIPELINE_PIPELINE_SCOPE_H_
#define CORE_RENDERER_PIPELINE_PIPELINE_SCOPE_H_

#include <memory>

#include "core/public/pipeline_option.h"

namespace lynx {
namespace tasm {
class TemplateAssembler;

class PipelineScope {
 public:
  PipelineScope(TemplateAssembler* tasm,
                const std::shared_ptr<PipelineOptions>& pipeline_options);
  PipelineScope(TemplateAssembler* tasm,
                const std::shared_ptr<PipelineOptions>& pipeline_options,
                bool is_major_updated);
  ~PipelineScope();
  PipelineScope(const PipelineScope&) = delete;
  PipelineScope& operator=(const PipelineScope&) = delete;
  PipelineScope(PipelineScope&&) = delete;
  PipelineScope& operator=(PipelineScope&&) = delete;

  void Exit();

 private:
  TemplateAssembler* tasm_{nullptr};
};
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RENDERER_PIPELINE_PIPELINE_SCOPE_H_
