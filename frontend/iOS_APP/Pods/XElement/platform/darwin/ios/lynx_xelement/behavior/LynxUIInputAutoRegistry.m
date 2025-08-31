// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Lynx/LynxComponentRegistry.h>
#import <Lynx/LynxUIOwner.h>
#import <XElement/LynxUIInputAutoRegistry.h>

@implementation LynxUIInputShadowNodeAutoRegistry

LYNX_LAZY_REGISTER_SHADOW_NODE("input")

@end

@implementation LynxUIInputAutoRegistry

LYNX_LAZY_REGISTER_UI("input")

@end
