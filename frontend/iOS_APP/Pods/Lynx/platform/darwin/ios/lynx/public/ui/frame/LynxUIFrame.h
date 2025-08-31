// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Lynx/LynxFrameView.h>
#import <Lynx/LynxTemplateBundle.h>
#import <Lynx/LynxUI.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxUIFrame : LynxUI <LynxFrameView*>

- (void)onReceiveAppBundle:(LynxTemplateBundle*)bundle;

@end

NS_ASSUME_NONNULL_END
