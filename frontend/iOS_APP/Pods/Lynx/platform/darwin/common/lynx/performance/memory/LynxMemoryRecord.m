// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LynxMemoryRecord.h"

@implementation LynxMemoryRecord

- (instancetype)initWithCategory:(NSString*)category
                       sizeBytes:(int64_t)sizeBytes
                          detail:(NSDictionary<NSString*, NSString*>* _Nullable)detail {
  if ([self init]) {
    _category = category;
    _sizeBytes = sizeBytes;
    _detail = detail;
  }
  return self;
}

@end
