// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import <Lynx/LynxDevtool.h>
#import <Lynx/LynxError.h>
#import <Lynx/LynxView.h>

NS_ASSUME_NONNULL_BEGIN

@protocol LynxLogBoxProtocol <NSObject>
@required

- (void)setLynxDevTool:(LynxDevtool *)devtool;

- (void)showLogMessage:(LynxError *)error;

- (void)onMovedToWindow;

- (void)attachLynxView:(nonnull LynxView *)lynxView;

- (void)onLynxViewReload;  // long press, Page.reload, etc

- (void)destroy;

@end

NS_ASSUME_NONNULL_END
