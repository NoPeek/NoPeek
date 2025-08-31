// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "core/renderer/css/parser/font_variation_settings_handler.h"

#include <string>
#include <utility>
#include <vector>

#include "base/include/debug/lynx_assert.h"
#include "base/include/value/array.h"
#include "core/renderer/css/parser/css_string_parser.h"
#include "core/renderer/css/unit_handler.h"

namespace lynx {
namespace tasm {
namespace FontVariationSettingsHandler {

HANDLER_IMPL() {
  CSS_HANDLER_FAIL_IF_NOT(input.IsString(), configs.enable_css_strict_mode,
                          TYPE_MUST_BE, CSSProperty::GetPropertyNameCStr(key),
                          STRING_TYPE)
  CSSStringParser parser = CSSStringParser::FromLepusString(input, configs);
  auto arr = lepus::CArray::Create();
  if (!parser.ParseFontVariationSettings(arr)) {
    return false;
  }
  output.emplace_or_assign(key, std::move(arr));
  return true;
}

HANDLER_REGISTER_IMPL() { array[kPropertyIDFontVariationSettings] = &Handle; }

}  // namespace FontVariationSettingsHandler
}  // namespace tasm
}  // namespace lynx
