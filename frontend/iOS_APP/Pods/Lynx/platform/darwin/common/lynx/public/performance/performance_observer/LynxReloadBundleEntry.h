// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxPipelineEntry.h"

@interface LynxReloadBundleEntry : LynxPipelineEntry
@property(nonatomic, strong) NSNumber* reloadBundleStart;
@property(nonatomic, strong) NSNumber* reloadBundleEnd;
@property(nonatomic, strong) NSNumber* reloadBackgroundStart;
@property(nonatomic, strong) NSNumber* reloadBackgroundEnd;
@property(nonatomic, strong) NSNumber* ffiStart;
@property(nonatomic, strong) NSNumber* ffiEnd;
- (instancetype)initWithDictionary:(NSDictionary*)dictionary;
@end
