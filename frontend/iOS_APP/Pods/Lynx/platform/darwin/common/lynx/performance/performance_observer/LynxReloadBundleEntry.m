// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxReloadBundleEntry.h"

@implementation LynxReloadBundleEntry

- (instancetype)initWithDictionary:(NSDictionary*)dictionary {
    self = [super initWithDictionary:dictionary];
    if (self) {
        self.reloadBundleStart = dictionary[@"reloadBundleStart"]?: @(-1);
        self.reloadBundleEnd = dictionary[@"reloadBundleEnd"]?: @(-1);
        self.reloadBackgroundStart = dictionary[@"reloadBackgroundStart"]?: @(-1);
        self.reloadBackgroundEnd = dictionary[@"reloadBackgroundEnd"]?: @(-1);
        self.ffiStart = dictionary[@"ffiStart"]?: @(-1);
        self.ffiEnd = dictionary[@"ffiEnd"]?: @(-1);
    }
    return self;
}

@end
