// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_RESOURCE_LAZY_BUNDLE_BUNDLE_RESOURCE_INFO_H_
#define CORE_RESOURCE_LAZY_BUNDLE_BUNDLE_RESOURCE_INFO_H_

#include <string>

#include "base/include/value/base_string.h"
#include "base/include/value/base_value.h"
#include "base/include/value/table.h"

namespace lynx {
namespace tasm {

static constexpr const char kBundleResourceInfoKeyUrl[] = "url";
static constexpr const char kBundleResourceInfoKeyCode[] = "code";
static constexpr const char kBundleResourceInfoKeyError[] = "error_msg";

const int32_t LYNX_BUNDLE_RESOURCE_INFO_SUCCESS = 0;
const int32_t LYNX_BUNDLE_RESOURCE_INFO_REQUEST_FAILED = -1;
const int32_t LYNX_BUNDLE_RESOURCE_INFO_TIMEOUT = -2;

struct BundleResourceInfo {
  /**
   * url of the bundle to be loaded;
   */
  std::string url;

  /**
   * status code of the bundle loading process;
   *  0: success;
   *  -1: request failed, retrieve error message from `error_msg` field;
   *  -2: request timeout;
   */
  int code;

  /**
   * error message if the bundle load failed;
   */
  std::string error_msg;

  lepus::Value ConvertToLepusValue() {
    auto result = lepus::Dictionary::Create();
    result->SetValue(BASE_STATIC_STRING(kBundleResourceInfoKeyUrl), url);
    result->SetValue(BASE_STATIC_STRING(kBundleResourceInfoKeyCode), code);
    result->SetValue(BASE_STATIC_STRING(kBundleResourceInfoKeyError),
                     error_msg);
    return lepus::Value(result);
  }
};
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RESOURCE_LAZY_BUNDLE_BUNDLE_RESOURCE_INFO_H_
