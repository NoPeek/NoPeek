// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_RENDERER_PIPELINE_PIPELINE_VERSION_H_
#define CORE_RENDERER_PIPELINE_PIPELINE_VERSION_H_

#include <atomic>
#include <cstdint>
#include <string>

namespace lynx {
namespace tasm {
class PipelineVersion {
 public:
  ~PipelineVersion() = default;
  static PipelineVersion Create() { return PipelineVersion(0, 0); };

  bool operator<(const PipelineVersion& other) const {
    if (major_ != other.major_) {
      return major_ < other.major_;
    }
    return minor_ < other.minor_;
  }

  bool operator==(const PipelineVersion& other) const {
    return major_ == other.major_ && minor_ == other.minor_;
  }

  PipelineVersion GenerateNextMinorVersion() const {
    return PipelineVersion(major_, minor_ + 1);
  }

  PipelineVersion GenerateNextMajorVersion() const {
    return PipelineVersion(major_ + 1, minor_);
  }

  int64_t GetMajor() const { return major_; }
  int64_t GetMinor() const { return minor_; }

  const std::string ToString() const {
    return std::to_string(major_) + "." + std::to_string(minor_);
  }

 private:
  PipelineVersion(int64_t major, int64_t minor)
      : major_(major), minor_(minor){};

  int64_t major_{0};
  int64_t minor_{0};
};
}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RENDERER_PIPELINE_PIPELINE_VERSION_H_
