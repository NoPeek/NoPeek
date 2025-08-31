// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XElement/LynxNumberKeyListener.h>
#import <Foundation/Foundation.h>

@implementation LynxNumberKeyListener

- (instancetype)init {
    self = [super init];
    return self;
}

- (NSInteger)getInputType {
    NSInteger type = TYPE_CLASS_NUMBER;
    return type;
}

- (NSString *)getAcceptedChars {
    // empty implementation
    return @"";
}

- (BOOL)checkCharIsInCharacterSet:(NSString*)characterSet character:(unichar)ch {
    for (int i = (int)[characterSet length] - 1; i >= 0; i--) {
        if (ch == [characterSet characterAtIndex:i]) {
            return YES;
        }
    }
    return NO;
}

- (NSString *)filter:(NSString *)source start:(NSInteger)start end:(NSInteger)end dest:(NSString *)dest dstart:(NSInteger)dstart dend:(NSInteger)dend {
    NSMutableString* filteredText = [NSMutableString stringWithString:source];
    NSString* accepted = [self getAcceptedChars];

    if (filteredText != nil) {
        for (int i = (int)[source length] - 1; i >= 0; i--) {
            if (![self checkCharIsInCharacterSet:accepted character:[source characterAtIndex:i]]) {
                [filteredText deleteCharactersInRange:NSMakeRange(i, 1)];
            }
        }
    } else {
        return @"";
    }
    
    return filteredText;
}
@end
