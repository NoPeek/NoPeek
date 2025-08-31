// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XElement/LynxKeyListener.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxNumberKeyListener : NSObject <LynxKeyListener>

- (NSInteger)getInputType;
- (NSString*)getAcceptedChars;
- (NSString*)filter:(NSString*)source start:(NSInteger)start end:(NSInteger)end dest:(NSString*)dest dstart:(NSInteger)dstart dend:(NSInteger)dend;
- (BOOL)checkCharIsInCharacterSet:(NSString*)characterSet character:(unichar)ch;

@end

NS_ASSUME_NONNULL_END
