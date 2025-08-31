//
//  LynxProvider.m
//  PhotoSafer
//
//  Created by 李奥 on 2025/8/31.
//

#import "LynxProvider.h"

@implementation LynxProvider

- (void)loadTemplateWithUrl:(NSString*)url onComplete:(LynxTemplateLoadBlock)callback {
    NSString *filePath = [[NSBundle mainBundle] pathForResource:url ofType:@"bundle"];
    if (filePath) {
        NSError *error;
        NSData *data = [NSData dataWithContentsOfFile:filePath options:0 error:&error];
        if (error) {
            NSLog(@"Error reading file: %@", error.localizedDescription);
            callback(nil, error);
        } else {
            callback(data, nil);
        }
    } else {
        NSError *urlError = [NSError errorWithDomain:@"com.lynx"
                                                code:400
                                            userInfo:@{NSLocalizedDescriptionKey : @"Invalid URL."}];
        callback(nil, urlError);
    }
}

@end
