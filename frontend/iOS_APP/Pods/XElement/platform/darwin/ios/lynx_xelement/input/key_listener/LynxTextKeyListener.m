// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XElement/LynxTextKeyListener.h>
#import <Foundation/Foundation.h>

@implementation LynxTextKeyListener

- (instancetype)init {
  if (self = [super init]) {
  }
  return self;
}

- (NSInteger)getInputType {
  return TYPE_CLASS_TEXT;
}

- (NSString *)filter:(NSString *)source start:(NSInteger)start end:(NSInteger)end dest:(NSString *)dest dstart:(NSInteger)dstart dend:(NSInteger)dend {
    return source;
}

@end
