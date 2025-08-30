// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

//
//  LynxPhotoSaver.h
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/27.
//

#import <Lynx/LynxUI.h>
#import <UIKit/UIKit.h>
#import <Photos/Photos.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxPhotoSaverView : UIView

@end

@interface LynxPhotoSaver : LynxUI <LynxPhotoSaverView *> <PHPhotoLibraryChangeObserver>

@end

NS_ASSUME_NONNULL_END
