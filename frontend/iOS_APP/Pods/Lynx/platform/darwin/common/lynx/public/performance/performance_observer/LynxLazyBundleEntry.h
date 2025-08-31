// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxPerformanceEntry.h"

@interface LynxLazyBundleEntry : LynxPerformanceEntry
@property(nonatomic, strong) NSString* componentUrl;
@property(nonatomic, strong) NSString* mode;
@property(nonatomic, strong) NSNumber* size;
@property(nonatomic, assign) BOOL sync;
@property(nonatomic, assign) BOOL loadSuccess;
@property(nonatomic, strong) NSNumber* requireStart;
@property(nonatomic, strong) NSNumber* requireEnd;
@property(nonatomic, strong) NSNumber* decodeStart;
@property(nonatomic, strong) NSNumber* decodeEnd;
- (instancetype)initWithDictionary:(NSDictionary*)dictionary;
@end
