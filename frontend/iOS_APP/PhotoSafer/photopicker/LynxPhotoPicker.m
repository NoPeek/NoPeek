//
//  LynxPhotoPicker.m
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/27.
//

#import "LynxPhotoPicker.h"
#import <Lynx/LynxComponentRegistry.h>
#import <Lynx/LynxPropsProcessor.h>
#import <Lynx/LynxUIOwner.h>
#import <Photos/Photos.h>
#import <MobileCoreServices/MobileCoreServices.h>

@implementation LynxPhotoPickerView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    return self;
}

@end

@implementation LynxPhotoPicker

- (instancetype)init {
  self = [super init];
  if (self) {
    _maxImageWidth = 1024;
    _maxImageHeight = 1024;
    _imageQuality = 0.8;
  }
  return self;
}

- (UIView *)createView {
  NSLog(@"PhotoPicker createView called");
  return [[LynxPhotoPickerView alloc] init];
}

- (void)layoutDidFinished {
  [super layoutDidFinished];
  NSLog(@"PhotoPicker layoutDidFinished called");
}

LYNX_PROP_SETTER("max-width", setMaxWidth, NSNumber *) {
  self.maxImageWidth = [value floatValue];
}

LYNX_PROP_SETTER("max-height", setMaxHeight, NSNumber *) {
  self.maxImageHeight = [value floatValue];
}

LYNX_PROP_SETTER("quality", setQuality, NSNumber *) {
  CGFloat quality = [value floatValue];
  if (quality >= 0.0 && quality <= 1.0) {
    self.imageQuality = quality;
  }
}

LYNX_LAZY_REGISTER_UI("photo-picker")

// Method for Lynx
LYNX_UI_METHOD(openPicker) {
  NSLog(@"PhotoPicker openPicker method called");
  
  [self requestPhotoAuthorizationWithCompletion:^(BOOL granted) {
    NSLog(@"PhotoPicker authorization granted: %d", granted);
    if (granted) {
      [self presentImagePickerController];
      callback(kUIMethodSuccess, nil);
    } else {
      callback(kUIMethodInvalidStateError, @"Access to Photos was rejected");
    }
  }];
}

- (void)requestPhotoAuthorizationWithCompletion:(void (^)(BOOL))completion {
  NSLog(@"PhotoPicker requesting photo authorization");
  PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
  NSLog(@"PhotoPicker current authorization status: %ld", (long)status);
  
  if (status == PHAuthorizationStatusAuthorized) {
    NSLog(@"PhotoPicker already authorized");
    completion(YES);
  } else if (status == PHAuthorizationStatusNotDetermined) {
    NSLog(@"PhotoPicker requesting authorization");
    [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
      dispatch_async(dispatch_get_main_queue(), ^{
        BOOL isAuthorized = (status == PHAuthorizationStatusAuthorized);
        NSLog(@"PhotoPicker authorization result: %d", isAuthorized);
        completion(isAuthorized);
      });
    }];
  } else {
    NSLog(@"PhotoPicker authorization denied");
    completion(NO);
  }
}

- (void)presentImagePickerController {
  NSLog(@"PhotoPicker presenting image picker controller");
  
  if (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypePhotoLibrary]) {
    NSLog(@"PhotoPicker photo library not available");
    [self emitEvent:@"error" detail:@{@"message": @"The device doesn't support Photos"}];
    return;
  }
  
  UIImagePickerController *picker = [[UIImagePickerController alloc] init];
  picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
  picker.mediaTypes = @[(NSString *)kUTTypeImage];
  picker.delegate = self;
  
  UIViewController *rootVC = [UIApplication sharedApplication].keyWindow.rootViewController;
  
  NSLog(@"PhotoPicker about to present picker, rootViewController: %@", rootVC);
  
  if (rootVC) {
    [rootVC presentViewController:picker animated:YES completion:^{
      NSLog(@"PhotoPicker picker presented successfully");
    }];
  } else {
    NSLog(@"PhotoPicker rootViewController is nil");
    [self emitEvent:@"error" detail:@{@"message": @"Cannot get rootViewController"}];
  }
}

#pragma mark - UIImagePickerControllerDelegate

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey,id> *)info {
  NSLog(@"PhotoPicker image selected");
  UIImage *image = info[UIImagePickerControllerOriginalImage];
  UIImage *resizedImage = [self resizeImage:image maxWidth:self.maxImageWidth maxHeight:self.maxImageHeight];
  
  if (resizedImage == image) {
    NSLog(@"PhotoPicker: WARNING - resizedImage is the same object as original image");
  }
  
  NSData *imageData = UIImageJPEGRepresentation(resizedImage, self.imageQuality);
  NSString *base64String = [NSString stringWithFormat:@"data:image/jpeg;base64,%@", [imageData base64EncodedStringWithOptions:0]];
  
  base64String = [base64String stringByReplacingOccurrencesOfString:@"\n" withString:@""];
  base64String = [base64String stringByReplacingOccurrencesOfString:@"\r" withString:@""];
  base64String = [base64String stringByReplacingOccurrencesOfString:@" " withString:@""];
  
  NSLog(@"PhotoPicker: JPEG data size: %lu bytes", (unsigned long)imageData.length);
  NSLog(@"PhotoPicker: base64 string length: %lu", (unsigned long)base64String.length);
  
  [self emitEvent:@"change" detail:@{
    @"image": base64String,
    @"width": @(resizedImage.size.width),
    @"height": @(resizedImage.size.height)
  }];
  NSLog(@"PhotoPicker emitting change event with image");
  
  [picker dismissViewControllerAnimated:YES completion:nil];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
  NSLog(@"PhotoPicker image selection cancelled");
  [self emitEvent:@"cancel" detail:@{}];
  [picker dismissViewControllerAnimated:YES completion:nil];
}

- (void)emitEvent:(NSString *)name detail:(NSDictionary *)detail {
  NSLog(@"PhotoPicker emitting event: %@, detail: %@", name, detail);
  LynxCustomEvent *eventInfo = [[LynxDetailEvent alloc] initWithName:name
                                                          targetSign:[self sign]
                                                              detail:detail];
  [self.context.eventEmitter dispatchCustomEvent:eventInfo];
}

- (UIImage *)resizeImage:(UIImage *)image maxWidth:(CGFloat)maxWidth maxHeight:(CGFloat)maxHeight {
  if (!image) {
    NSLog(@"PhotoPicker: resizeImage called with nil image");
    return nil;
  }
  
  CGSize imageSize = image.size;
  NSLog(@"PhotoPicker: original image size: %.0fx%.0f", imageSize.width, imageSize.height);
  NSLog(@"PhotoPicker: max size: %.0fx%.0f", maxWidth, maxHeight);
  
  CGFloat width = imageSize.width;
  CGFloat height = imageSize.height;
  
  CGFloat scale = 1.0;
  
  if (width > maxWidth || height > maxHeight) {
    CGFloat widthScale = maxWidth / width;
    CGFloat heightScale = maxHeight / height;
    
    scale = fmin(widthScale, heightScale);
    NSLog(@"PhotoPicker: widthScale: %.2f, heightScale: %.2f, using scale: %.2f", widthScale, heightScale, scale);
  } else {
    NSLog(@"PhotoPicker: image is smaller than max size, no scaling needed");
  }
  
  CGSize newSize = CGSizeMake(width * scale, height * scale);
  NSLog(@"PhotoPicker: new image size: %.0fx%.0f", newSize.width, newSize.height);
  
  if (scale >= 1.0) {
    NSLog(@"PhotoPicker: scale >= 1.0, returning original image");
    return image;
  }
  
  UIGraphicsImageRendererFormat *format = [UIGraphicsImageRendererFormat defaultFormat];
  format.scale = image.scale;
  format.opaque = YES;
  
  UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:newSize format:format];
  
  UIImage *resizedImage = [renderer imageWithActions:^(UIGraphicsImageRendererContext * _Nonnull context) {
    [image drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
  }];
  
  if (resizedImage) {
    NSLog(@"PhotoPicker: successfully resized image");
    return resizedImage;
  } else {
    NSLog(@"PhotoPicker: failed to resize image, returning original");
    return image;
  }
}

@end
