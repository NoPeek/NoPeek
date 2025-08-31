// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "core/renderer/pipeline/pipeline_scope.h"

#include "core/renderer/template_assembler.h"

namespace lynx {
namespace tasm {
PipelineScope::PipelineScope(
    TemplateAssembler* tasm,
    const std::shared_ptr<PipelineOptions>& pipeline_options)
    : PipelineScope(tasm, pipeline_options, false) {}

PipelineScope::PipelineScope(
    TemplateAssembler* tasm,
    const std::shared_ptr<PipelineOptions>& pipeline_options,
    bool is_major_updated) {
  if (!tasm) {
    return;
  }

  tasm_ = tasm;
  tasm_->CreateAndUpdateCurrentPipelineContext(pipeline_options,
                                               is_major_updated);
}

PipelineScope::~PipelineScope() { Exit(); }

void PipelineScope::Exit() {
  if (!tasm_) {
    return;
  }
  tasm_->RunPixelPipeline();
  tasm_ = nullptr;
}
}  // namespace tasm
}  // namespace lynx
