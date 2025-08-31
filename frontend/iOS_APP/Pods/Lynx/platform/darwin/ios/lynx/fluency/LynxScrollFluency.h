// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PLATFORM_DARWIN_IOS_LYNX_FLUENCY_LYNXSCROLLFLUENCY_H_
#define PLATFORM_DARWIN_IOS_LYNX_FLUENCY_LYNXSCROLLFLUENCY_H_

#import <Lynx/LynxScrollListener.h>
#import "LynxFluencyMonitor.h"

NS_ASSUME_NONNULL_BEGIN

@interface LynxScrollFluency : NSObject <LynxScrollListener>

- (void)setEnabledBySampling:(LynxBooleanOption)enabledBySampling;

- (void)setPageConfigProbability:(CGFloat)probability;

- (BOOL)shouldSendAllScrollEvent;

@end

NS_ASSUME_NONNULL_END

#endif  // PLATFORM_DARWIN_IOS_LYNX_FLUENCY_LYNXSCROLLFLUENCY_H_
