// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Lynx/LynxTemplateRender.h>

/**
 * internal class helping to setup UIRender, LynxShell and Event
 */
@interface LynxTemplateRender (Helper)

- (void)setUpWithBuilder:(LynxViewBuilder*)builder screenSize:(CGSize)screenSize;

- (void)reset:(int32_t)lastInstanceId;

@end
