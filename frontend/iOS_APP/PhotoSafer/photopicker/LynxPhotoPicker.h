//
//  LynxPhotoPicker.h
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/27.
//

#import <Lynx/LynxUI.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface LynxPhotoPickerView : UIView

@end

// 移除内部按钮，只保留图片显示区域
@interface LynxPhotoPicker : LynxUI <LynxPhotoPickerView *> <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

@property (nonatomic, assign) CGFloat maxImageWidth;
@property (nonatomic, assign) CGFloat maxImageHeight;
@property (nonatomic, assign) CGFloat imageQuality; // 0.0-1.0

@end

NS_ASSUME_NONNULL_END
