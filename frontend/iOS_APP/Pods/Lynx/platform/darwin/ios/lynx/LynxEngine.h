// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>
#import <Lynx/LynxTemplateRender.h>
#include "core/shell/lynx_engine_wrapper.h"

@class LynxTemplateRender;
@class LynxShadowNodeOwner;
@class LynxTemplateBundle;

@protocol LynxUIRendererProtocol;

typedef NS_ENUM(NSInteger, LynxEngineState) {
  LynxEngineStateUnloaded,
  LynxEngineStateReadyToBeReused,
  LynxEngineStateOnReusing,
  LynxEngineStateDestroyed
};

NS_ASSUME_NONNULL_BEGIN

@interface LynxEngine : NSObject

@property(nonatomic, weak) LynxTemplateRender *lynxTemplateRender;
@property(nonatomic, strong) LynxTemplateBundle *templateBundle;
@property(nonatomic, assign) LynxEngineState engineState;
@property(nonatomic, strong, nullable) id<LynxUIRendererProtocol> lynxUIRenderer;
@property(nonatomic, strong, nullable) LynxShadowNodeOwner *shadowNodeOwner;
@property(nonatomic, weak) NSMutableArray<LynxEngine *> *engineQueueRef;

- (instancetype)initWithTemplateRender:(LynxTemplateRender *)render;
- (lynx::shell::LynxEngineWrapper *)getEngineNative;
- (BOOL)hasLoaded;
- (BOOL)canBeReused;
- (void)registerToReuse;
- (BOOL)isRunOnCurrentTemplateRender:(LynxTemplateRender *)render;
- (void)attachTemplateRender:(LynxTemplateRender *)render;
- (void)detachEngine;
- (void)destroy;

@end

NS_ASSUME_NONNULL_END
