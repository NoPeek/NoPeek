// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>
#import "LynxEngine.h"

@class LynxEngine;
@class LynxTemplateRender;

NS_ASSUME_NONNULL_BEGIN

@interface LynxEnginePool : NSObject

+ (instancetype)sharedInstance;

- (void)registerReuseEngine:(LynxEngine *)engine;

- (LynxEngine *)pollEngineWithRender:(LynxTemplateBundle *)render;

@end

NS_ASSUME_NONNULL_END
