// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxPerformanceEntry.h"
#import "LynxMemoryUsageItem.h"

@interface LynxMemoryUsageEntry : LynxPerformanceEntry
@property(nonatomic, strong) NSNumber* sizeBytes;
@property(nonatomic, strong) NSDictionary<NSString*, LynxMemoryUsageItem*>* detail;
- (instancetype)initWithDictionary:(NSDictionary*)dictionary;
@end
