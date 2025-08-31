// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>
#import "LynxMetricFcpEntry.h"
#import "LynxPerformanceMetric.h"

@implementation LynxMetricFcpEntry

- (instancetype)initWithDictionary:(NSDictionary*)dictionary {
    self = [super initWithDictionary:dictionary];
    if (self) {
        self.fcp = dictionary[@"fcp"] ? [[LynxPerformanceMetric alloc] initWithDictionary:dictionary[@"fcp"]] : [[LynxPerformanceMetric alloc] initWithDictionary:@{}];
        self.lynxFcp = dictionary[@"lynxFcp"] ? [[LynxPerformanceMetric alloc] initWithDictionary:dictionary[@"lynxFcp"]] : [[LynxPerformanceMetric alloc] initWithDictionary:@{}];
        self.totalFcp = dictionary[@"totalFcp"] ? [[LynxPerformanceMetric alloc] initWithDictionary:dictionary[@"totalFcp"]] : [[LynxPerformanceMetric alloc] initWithDictionary:@{}];
    }
    return self;
}

@end
