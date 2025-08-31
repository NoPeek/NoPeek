// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XElement/LynxNumberKeyListener.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxDialerKeyListener : LynxNumberKeyListener

@property(nonatomic, readonly) NSArray<NSString*>* CHARACTERS;

- (NSInteger)getInputType;
- (NSString*)getAcceptedChars;

@end

NS_ASSUME_NONNULL_END
