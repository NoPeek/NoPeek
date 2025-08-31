// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LynxGestureDetectorDarwin;
// Used to let lynx native gesture block the scrolling of UIScrollView

typedef NS_ENUM(NSInteger, LynxGestureConsumeStatus) {
  LynxGestureConsumeStatusUndefined = -1,
  LynxGestureConsumeStatusBlock = 0,
  LynxGestureConsumeStatusAllow = 1,
};

typedef NS_ENUM(NSInteger, LynxInterceptGestureState) {
  LynxInterceptGestureStateUnset = 0,
  LynxInterceptGestureStateFalse = 1,
  LynxInterceptGestureStateTrue = 2
};

@interface LynxGestureConsumer : NSObject

@property(nonatomic, assign) BOOL adjustingScrollOffset;
@property(nonatomic, assign, readonly) LynxGestureConsumeStatus gestureConsumeStatus;
@property(nonatomic, assign) LynxInterceptGestureState interceptGestureStatus;

@property(nonatomic, assign) CGPoint previousScrollOffset;

- (void)consumeGesture:(BOOL)consume;

/**
 * @breif Dynamically intercepting native gestures
 * @param intercept true: intercept native gesture, false: not intercept native gesture
 */
- (void)interceptGesture:(BOOL)intercept;

@end

@interface UIScrollView (LynxGesture)

- (BOOL)respondToScrollViewDidScroll:(LynxGestureConsumer *)gestureConsumer;

- (void)disableGesturesRecursivelyIfNecessary:(LynxGestureConsumer *)gestureConsumer;

- (BOOL)stopDeceleratingIfNecessaryWithTargetContentOffset:(inout CGPoint *)targetContentOffset;

@end

NS_ASSUME_NONNULL_END
