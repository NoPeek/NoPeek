// Copyright 2019 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef CORE_RENDERER_DOM_VDOM_RADON_RADON_TYPES_H_
#define CORE_RENDERER_DOM_VDOM_RADON_RADON_TYPES_H_

#include <cstdint>

namespace lynx {
namespace tasm {

enum RadonNodeType : int8_t {
  kRadonUnknown = -1,
  kRadonNode = 0,
  kRadonComponent,
  kRadonPage,
  kRadonSlot,
  kRadonPlug,
  kRadonListNode,
  kRadonBlock,
  kRadonLazyComponent,
};

constexpr static const char kRadonComponentTag[] = "component";
constexpr static const char kRadonPageTag[] = "page";

}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RENDERER_DOM_VDOM_RADON_RADON_TYPES_H_
