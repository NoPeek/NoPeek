// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxLazyBundleEntry.h"

@implementation LynxLazyBundleEntry

- (instancetype)initWithDictionary:(NSDictionary*)dictionary {
    self = [super initWithDictionary:dictionary];
    if (self) {
        self.componentUrl = dictionary[@"componentUrl"]?: @"";
        self.mode = dictionary[@"mode"]?: @"";
        self.size = dictionary[@"size"]?: @(-1);
        self.sync = [dictionary[@"sync"] boolValue];
        self.loadSuccess = [dictionary[@"loadSuccess"] boolValue];
        self.requireStart = dictionary[@"requireStart"]?: @(-1);
        self.requireEnd = dictionary[@"requireEnd"]?: @(-1);
        self.decodeStart = dictionary[@"decodeStart"]?: @(-1);
        self.decodeEnd = dictionary[@"decodeEnd"]?: @(-1);
    }
    return self;
}

@end
