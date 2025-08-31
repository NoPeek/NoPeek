// Inspired by S.js by Adam Haile, https://github.com/adamhaile/S
/**
The MIT License (MIT)

Copyright (c) 2017 Adam Haile

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef CORE_RENDERER_SIGNAL_SIGNAL_CONTEXT_H_
#define CORE_RENDERER_SIGNAL_SIGNAL_CONTEXT_H_

#include <list>
#include <memory>
#include <unordered_set>

#include "base/include/fml/memory/ref_counted.h"
#include "base/include/vector.h"
#include "core/renderer/signal/scope.h"

namespace lynx {
namespace tasm {

class Computation;
class Scope;

class SignalContext {
 public:
  SignalContext();

  void PushScope(BaseScope* scope);

  void PopScope();

  BaseScope* GetTopScope();

  void PushComputation(Computation* scope);

  void PopComputation();

  Computation* GetTopComputation();

  void MarkUnTrack(bool enable_un_track);

  void RunUpdates(std::function<void()>&& func);

  void CompleteUpdates(bool wait);

  void EnqueueComputation(Computation* computation);

  void RunComputation(Computation* computation);

  void RunComputation(
      std::shared_ptr<std::list<fml::RefPtr<Computation>>>&& list);

  void UpdateComputation(Computation* computation);

  void WillDestroy();

  void RecordScope(Scope* scope);

  void EraseScope(Scope* scope);

 private:
  void EnsurePureComputationList();
  void EnsureMemoComputationList();

  bool IsScopeActiveComputation(BaseScope* scope);

  int32_t exec_count_{0};

  base::Vector<BaseScope*> scope_stack_;

  base::Vector<Computation*> computation_stack_;

  std::shared_ptr<std::list<fml::RefPtr<Computation>>>
      pure_computation_list_ptr_;

  std::shared_ptr<std::list<fml::RefPtr<Computation>>>
      memo_computation_list_ptr_;

  std::unordered_set<Scope*> scope_set_;
};

}  // namespace tasm
}  // namespace lynx

#endif  // CORE_RENDERER_SIGNAL_SIGNAL_CONTEXT_H_
