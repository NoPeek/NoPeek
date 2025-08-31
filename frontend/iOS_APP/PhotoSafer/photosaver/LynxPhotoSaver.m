//
//  PhotoSaver.m
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/27.
//

#import "LynxPhotoSaver.h"
#import <Lynx/LynxComponentRegistry.h>
#import <Lynx/LynxPropsProcessor.h>
#import <Lynx/LynxUIOwner.h>

@implementation LynxPhotoSaverView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    return self;
}

@end

@implementation LynxPhotoSaver

- (UIView *)createView {
    return [[LynxPhotoSaverView alloc] init];
}

- (void)layoutDidFinished {
    [super layoutDidFinished];
}

LYNX_LAZY_REGISTER_UI("photo-saver")

#pragma mark - Public Methods

LYNX_UI_METHOD(saveImage) {
    if (!params || ![params isKindOfClass:[NSDictionary class]]) {
        callback(kUIMethodParamInvalid, @"Wrong parameter");
        return;
    }
    
    NSString *base64String = params[@"image"];
    NSString *fileName = params[@"fileName"] ?: @"photo";
    
    if (!base64String || ![base64String isKindOfClass:[NSString class]]) {
        callback(kUIMethodParamInvalid, @"No image data passed");
        return;
    }
    
    [self saveBase64Image:base64String fileName:fileName completion:^(BOOL success, NSString *errorMessage) {
        if (success) {
            callback(kUIMethodSuccess, @{@"message": @"Save image Successfully"});
        } else {
            callback(kUIMethodUnknown, errorMessage ?: @"Failed to save image");
        }
    }];
}

#pragma mark - Private Methods

- (void)saveBase64Image:(NSString *)base64String fileName:(NSString *)fileName completion:(void (^)(BOOL success, NSString *errorMessage))completion {
    NSString *cleanBase64 = [self cleanBase64String:base64String];
    
    NSData *imageData = [[NSData alloc] initWithBase64EncodedString:cleanBase64 options:0];
    if (!imageData) {
        completion(NO, @"Failed to decode Base64 data");
        return;
    }
    
    UIImage *image = [UIImage imageWithData:imageData];
    if (!image) {
        completion(NO, @"Failed to create UIImage from image data");
        return;
    }
  
    NSData *compressedImageData = UIImageJPEGRepresentation(image, 0.8);
    if (!compressedImageData) {
        if (completion) {
            completion(NO, @"Failed to compress image");
        }
        return;
    }
  
    UIImage *compressedImage = [UIImage imageWithData:compressedImageData];
    if (!compressedImage) {
        if (completion) {
            completion(NO, @"Failed to create compressed UIImage");
        }
        return;
    }
    
    // Request for authorization
    [self requestPhotoLibraryAuthorizationWithCompletion:^(BOOL granted) {
        if (!granted) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(NO, @"Access to PhotoLibrary denied");
            });
            return;
        }
      
      UIImageWriteToSavedPhotosAlbum(compressedImage, self, @selector(image:didFinishSavingWithError:contextInfo:), (__bridge void *)self);
    }];
}

- (NSString *)cleanBase64String:(NSString *)base64String {
    NSString *cleanString = base64String;
    NSRange prefixRange = [cleanString rangeOfString:@"base64,"];
    if (prefixRange.location != NSNotFound) {
        cleanString = [cleanString substringFromIndex:prefixRange.location + prefixRange.length];
    }
    
    cleanString = [cleanString stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    cleanString = [cleanString stringByReplacingOccurrencesOfString:@"\r" withString:@""];
    cleanString = [cleanString stringByReplacingOccurrencesOfString:@" " withString:@""];
    cleanString = [cleanString stringByReplacingOccurrencesOfString:@"\t" withString:@""];
    
    return cleanString;
}

- (void)requestPhotoLibraryAuthorizationWithCompletion:(void (^)(BOOL))completion {
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    
    switch (status) {
        case PHAuthorizationStatusAuthorized: {
            completion(YES);
            break;
        }
        case PHAuthorizationStatusNotDetermined: {
            [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    completion(status == PHAuthorizationStatusAuthorized);
                });
            }];
            break;
        }
        case PHAuthorizationStatusDenied:
        case PHAuthorizationStatusRestricted:
            completion(NO);
            break;
        default:
            completion(NO);
            break;
    }
}

#pragma mark - UIImageWriteToSavedPhotosAlbum Callback

- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (error) {
            NSLog(@"Failed to save image: %@", error.localizedDescription);
            [self emitEvent:@"failed" detail:@{
                @"message": error.localizedDescription ?: @"Failed to save photo for unknown error"
            }];
        } else {
            NSLog(@"Save image successfully");
            [self emitEvent:@"success" detail:@{
                @"message": @"Success"
            }];
        }
    });
}

#pragma mark - Event Emission

- (void)emitEvent:(NSString *)name detail:(NSDictionary *)detail {
    LynxCustomEvent *eventInfo = [[LynxDetailEvent alloc] initWithName:name
                                                            targetSign:[self sign]
                                                                detail:detail];
    [self.context.eventEmitter dispatchCustomEvent:eventInfo];
}

#pragma mark - PHPhotoLibraryChangeObserver
- (void)photoLibraryDidChange:(PHChange *)changeInstance {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self emitEvent:@"libraryChanged" detail:@{}];
    });
}

- (void)dealloc {
    // [[PHPhotoLibrary sharedPhotoLibrary] unregisterChangeObserver:self];
}

@end
