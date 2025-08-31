// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_RENDERER_DOM_FIBER_FRAME_ELEMENT_H_
#define CORE_RENDERER_DOM_FIBER_FRAME_ELEMENT_H_

#include <memory>
#include <string>

#include "core/renderer/dom/element_manager.h"
#include "core/renderer/dom/fiber/fiber_element.h"
#include "core/template_bundle/lynx_template_bundle.h"

namespace lynx {
namespace tasm {
class FrameElement : public FiberElement {
 public:
  explicit FrameElement(ElementManager* element_manager);
  ~FrameElement() override;

  void SetAttribute(const base::String& key, const lepus::Value& value,
                    bool need_update_data_model = true) override;

  bool DidBundleLoaded(const std::string& src,
                       const std::shared_ptr<LynxTemplateBundle>& bundle);

  void FlushProps() override;

 protected:
  void OnNodeAdded(FiberElement* child) override;

 private:
  // post bundle to UI node
  void PostBundle(const std::shared_ptr<LynxTemplateBundle>& bundle);

  // load bundle if src is set
  void OnSetSrc(const base::String& key, const lepus::Value& value);

  std::shared_ptr<LynxTemplateBundle> template_bundle_{nullptr};
  std::string src_{};
};
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RENDERER_DOM_FIBER_FRAME_ELEMENT_H_
