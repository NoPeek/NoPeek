//
//  LynxGestureView.m
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/27.
//

#import "LynxGestureRecognizer.h"
#import <Lynx/LynxComponentRegistry.h>
#import <Lynx/LynxPropsProcessor.h>
#import <Lynx/LynxUIOwner.h>

@implementation LynxGestureView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        self.userInteractionEnabled = YES;
        NSLog(@"LynxGestureView initWithFrame: %@", NSStringFromCGRect(frame));
    }
    return self;
}

- (void)awakeFromNib {
    [super awakeFromNib];
    NSLog(@"LynxGestureView awakeFromNib");
}

- (void)didMoveToSuperview {
    [super didMoveToSuperview];
    NSLog(@"LynxGestureView didMoveToSuperview, superview: %@", self.superview);
}

@end

@implementation LynxGestureRecognizer

LYNX_LAZY_REGISTER_UI("gesture-recognizer")

- (UIView *)createView {
    NSLog(@"LynxGestureRecognizer createView called");
    LynxGestureView *view = [[LynxGestureView alloc] init];
    NSLog(@"LynxGestureRecognizer view created: %@", view);
    return view;
}

- (void)layoutDidFinished {
    [super layoutDidFinished];
    NSLog(@"LynxGestureRecognizer layoutDidFinished called, view frame: %@", NSStringFromCGRect(self.view.frame));
    NSLog(@"LynxGestureRecognizer view superview: %@", self.view.superview);
    
    [self setupGestures];
    
    // 自动调整大小以填充父容器
    if (self.view.superview) {
        self.view.frame = self.view.superview.bounds;
        self.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        NSLog(@"LynxGestureRecognizer resized to superview bounds: %@", NSStringFromCGRect(self.view.frame));
    }
}

- (void)setupGestures {
    NSLog(@"LynxGestureRecognizer setupGestures called");
    
    // Tap Gesture
    self.tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    [self.view addGestureRecognizer:self.tapGesture];
    NSLog(@"LynxGestureRecognizer tap gesture added");
    
    // Long Press Gesture
    self.longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
    [self.view addGestureRecognizer:self.longPressGesture];
    NSLog(@"LynxGestureRecognizer long press gesture added");
    
    // Pan Gesture
    self.panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePan:)];
    [self.view addGestureRecognizer:self.panGesture];
    NSLog(@"LynxGestureRecognizer pan gesture added");
    
    // Swipe Gestures
    UISwipeGestureRecognizer *swipeRight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleSwipe:)];
    swipeRight.direction = UISwipeGestureRecognizerDirectionRight;
    [self.view addGestureRecognizer:swipeRight];
    NSLog(@"LynxGestureRecognizer swipe right gesture added");
    
    UISwipeGestureRecognizer *swipeLeft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleSwipe:)];
    swipeLeft.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.view addGestureRecognizer:swipeLeft];
    NSLog(@"LynxGestureRecognizer swipe left gesture added");
    
    // Pinch Gesture
    self.pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [self.view addGestureRecognizer:self.pinchGesture];
    NSLog(@"LynxGestureRecognizer pinch gesture added");
    
    // Rotation Gesture
    self.rotationGesture = [[UIRotationGestureRecognizer alloc] initWithTarget:self action:@selector(handleRotation:)];
    [self.view addGestureRecognizer:self.rotationGesture];
    NSLog(@"LynxGestureRecognizer rotation gesture added");
    
    NSLog(@"LynxGestureRecognizer all gestures setup completed");
}

LYNX_PROP_SETTER("width", setWidth, NSNumber *) {
    NSLog(@"LynxGestureRecognizer setWidth called with value: %@", value);
    if (value && [value isKindOfClass:[NSNumber class]]) {
        CGRect frame = self.view.frame;
        frame.size.width = [value floatValue];
        self.view.frame = frame;
        NSLog(@"LynxGestureRecognizer width set, new frame: %@", NSStringFromCGRect(self.view.frame));
    }
}

LYNX_PROP_SETTER("height", setHeight, NSNumber *) {
    NSLog(@"LynxGestureRecognizer setHeight called with value: %@", value);
    if (value && [value isKindOfClass:[NSNumber class]]) {
        CGRect frame = self.view.frame;
        frame.size.height = [value floatValue];
        self.view.frame = frame;
        NSLog(@"LynxGestureRecognizer height set, new frame: %@", NSStringFromCGRect(self.view.frame));
    }
}

#pragma mark - Gesture Handlers

- (void)handleTap:(UITapGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handleTap called, state: %ld", (long)gesture.state);
    CGPoint location = [gesture locationInView:self.view];
    NSLog(@"LynxGestureRecognizer tap location: (%f, %f)", location.x, location.y);
    [self emitEvent:@"tap" detail:@{
        @"x": @(location.x),
        @"y": @(location.y),
        @"state": @(gesture.state)
    }];
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handleLongPress called, state: %ld", (long)gesture.state);
    if (gesture.state == UIGestureRecognizerStateBegan) {
        CGPoint location = [gesture locationInView:self.view];
        NSLog(@"LynxGestureRecognizer long press began at: (%f, %f)", location.x, location.y);
        [self emitEvent:@"longpress" detail:@{
            @"x": @(location.x),
            @"y": @(location.y),
            @"state": @(gesture.state)
        }];
    }
}

- (void)handlePan:(UIPanGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handlePan called, state: %ld", (long)gesture.state);
    CGPoint translation = [gesture translationInView:self.view];
    CGPoint velocity = [gesture velocityInView:self.view];
    CGPoint location = [gesture locationInView:self.view];
    
    [self emitEvent:@"pan" detail:@{
        @"x": @(location.x),
        @"y": @(location.y),
        @"translationX": @(translation.x),
        @"translationY": @(translation.y),
        @"velocityX": @(velocity.x),
        @"velocityY": @(velocity.y),
        @"state": @(gesture.state)
    }];
}

- (void)handleSwipe:(UISwipeGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handleSwipe called, direction: %lu, state: %ld", 
          (unsigned long)gesture.direction, (long)gesture.state);
    NSString *direction;
    switch (gesture.direction) {
        case UISwipeGestureRecognizerDirectionRight:
            direction = @"right";
            break;
        case UISwipeGestureRecognizerDirectionLeft:
            direction = @"left";
            break;
        case UISwipeGestureRecognizerDirectionUp:
            direction = @"up";
            break;
        case UISwipeGestureRecognizerDirectionDown:
            direction = @"down";
            break;
        default:
            direction = @"unknown";
            break;
    }
    
    [self emitEvent:@"swipe" detail:@{
        @"direction": direction,
        @"state": @(gesture.state)
    }];
}

- (void)handlePinch:(UIPinchGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handlePinch called, scale: %f, state: %ld", 
          gesture.scale, (long)gesture.state);
    CGPoint centerPoint = [gesture locationInView:self.view];
    NSLog(@"LynxGestureRecognizer pinch center: (%f, %f)", centerPoint.x, centerPoint.y);
      
    [self emitEvent:@"pinch" detail:@{
        @"scale": @(gesture.scale),
        @"velocity": @(gesture.velocity),
        @"state": @(gesture.state),
        @"centerX": @(centerPoint.x),
        @"centerY": @(centerPoint.y)
    }];
}

- (void)handleRotation:(UIRotationGestureRecognizer *)gesture {
    NSLog(@"LynxGestureRecognizer handleRotation called, rotation: %f, state: %ld", 
          gesture.rotation, (long)gesture.state);
    [self emitEvent:@"rotation" detail:@{
        @"rotation": @(gesture.rotation),
        @"velocity": @(gesture.velocity),
        @"state": @(gesture.state)
    }];
}

#pragma mark - Event Emission

- (void)emitEvent:(NSString *)name detail:(NSDictionary *)detail {
    NSLog(@"LynxGestureRecognizer emitEvent: %@, detail: %@", name, detail);
    LynxCustomEvent *eventInfo = [[LynxDetailEvent alloc] initWithName:name
                                                            targetSign:[self sign]
                                                                detail:detail];
    [self.context.eventEmitter dispatchCustomEvent:eventInfo];
}

#pragma mark - Props (可选的启用/禁用特定手势)

LYNX_PROP_SETTER("enableTap", setEnableTap, BOOL) {
    NSLog(@"LynxGestureRecognizer setEnableTap: %d", value);
    if (self.tapGesture) {
        self.tapGesture.enabled = value;
    }
}

LYNX_PROP_SETTER("enablePan", setEnablePan, BOOL) {
    NSLog(@"LynxGestureRecognizer setEnablePan: %d", value);
    if (self.panGesture) {
        self.panGesture.enabled = value;
    }
}

LYNX_PROP_SETTER("enablePinch", setEnablePinch, BOOL) {
    NSLog(@"LynxGestureRecognizer setEnablePinch: %d", value);
    if (self.pinchGesture) {
        self.pinchGesture.enabled = value;
    }
}

@end
