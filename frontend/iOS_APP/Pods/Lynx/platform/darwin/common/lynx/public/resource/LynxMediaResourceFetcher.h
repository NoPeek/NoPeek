// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DARWIN_COMMON_LYNX_RESOURCE_LYNXMEDIARESOURCEFETCHER_H_
#define DARWIN_COMMON_LYNX_RESOURCE_LYNXMEDIARESOURCEFETCHER_H_

#import <Lynx/LynxResourceRequest.h>
#import <Lynx/LynxResourceResponse.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^LynxMediaResourceCompletionBlock)(UIImage* _Nullable uiImage,
                                                 NSError* _Nullable error);

typedef NS_ENUM(NSInteger, LynxResourceOptionalBool) {
  LynxResourceOptionalBoolTrue,
  LynxResourceOptionalBoolFalse,
  LynxResourceOptionalBoolUndefined
};

/**
 * @apidoc
 * @brief `LynxMediaResourceFetcher` is defined inside LynxEngine and
 * injected from outside to implement the path redirection capability
 * of `Image` and other third-party resources.
 */
@protocol LynxMediaResourceFetcher <NSObject>

/**
 * @apidoc
 * @brief `LynxEngine` redirects the image path by calling this method internally,
 * and the return result is required to be of `String` type
 * @param request Request for the resource.
 * @param callback The target url.
 * @note This method must be implemented.
 */
@required
- (NSString* _Nonnull)shouldRedirectUrl:(LynxResourceRequest* _Nonnull)request;

/**
 * @apidoc
 * @brief `LynxEngine` internally calls this method to determine whether the
 * resource path exists on disk.
 *
 * @param url Input path.
 * @return TRUE if is a local path, FALSE if not a local path and UNDEFINED if not sure.
 * @note This method is optional to be implemented.
 */
@optional
- (LynxResourceOptionalBool)isLocalResource:(NSURL* _Nonnull)url;

/**
 * @apidoc
 * @brief `LynxEngine` will use this method to obtain the bitmap information of the image,
 * and the return content must be of `Closeable` type.
 *
 * @param request Request for the resource.
 * @param response Response with the needed drawable.
 * @note This method is optional to be implemented.
 */
@optional
- (dispatch_block_t)fetchUIImage:(LynxResourceRequest* _Nonnull)request
                      onComplete:(LynxMediaResourceCompletionBlock _Nonnull)response;

@end

NS_ASSUME_NONNULL_END

#endif  // DARWIN_COMMON_LYNX_RESOURCE_LYNXMEDIARESOURCEFETCHER_H_
