// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Lynx/LynxUI.h>
#import <Lynx/LynxCustomMeasureShadowNode.h>
#import <XElement/LynxUIBaseInputShadowNode.h>
#import <XElement/LynxUIBaseInput.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxUITextAreaShadowNode : LynxUIBaseInputShadowNode

@end

@interface LynxUITextArea : LynxUIBaseInput<UITextView *>

@end

NS_ASSUME_NONNULL_END
