// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XElement/LynxDialerKeyListener.h>
#import <Foundation/Foundation.h>

static NSString* const CHARACTERS = @"0123456789#*+_(),/N. ;";

@implementation LynxDialerKeyListener

- (instancetype)init {
  self = [super init];
  if (self) {
    //
  }
  return self;
}

- (NSInteger)getInputType {
  return TYPE_CLASS_PHONE;
}

- (NSString*)getAcceptedChars {
  return CHARACTERS;
}

@end
