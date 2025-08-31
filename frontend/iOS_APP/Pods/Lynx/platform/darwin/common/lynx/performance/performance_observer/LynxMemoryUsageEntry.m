// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxMemoryUsageEntry.h"
#import "LynxMemoryUsageItem.h"

@implementation LynxMemoryUsageEntry

- (instancetype)initWithDictionary:(NSDictionary*)dictionary {
    self = [super initWithDictionary:dictionary];
    if (self) {
        self.sizeBytes = dictionary[@"sizeBytes"]?: @(-1);
        NSDictionary *originMap = dictionary[@"detail"] ? : @{};
        NSMutableDictionary *details = [[NSMutableDictionary alloc] init];
        for (NSString *key in originMap) {
            NSDictionary *entryValue = originMap[key];
            LynxMemoryUsageItem *item = [[LynxMemoryUsageItem alloc] initWithDictionary:entryValue];
            [details setObject:item forKey:key];
        }
        self.detail = details;
    }
    return self;
}

@end
