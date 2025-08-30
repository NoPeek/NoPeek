// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { type PropsWithChildren, useRef } from "@lynx-js/react";
import type { CSSProperties, Touch, TouchEvent } from "@lynx-js/types";

interface TouchPoint {
    x: number;
    y: number;
};

interface CustomEvent {
    detail: {
        [key: string]: any;
    };
};

interface GestureState {
    isPinching: boolean;
    isSliding: boolean;
    initialDistance?: number;
    initialTouchPoints: TouchPoint[];
    lastTouchPoints: TouchPoint[];
    startTime: number;
};

/**
 * @deprecated This cannot detect multifinger gestures
 */
export default function GestureDetectorView(props: PropsWithChildren<{
    className?: string;
    style?: CSSProperties,
    onPinch?: (event: CustomEvent) => void,
    onPinchEnd?: () => void,
    onSlide?: (event: CustomEvent) => void,
    onSlideStart?: (event: CustomEvent) => void,
    onSlideEnd?: () => void,
}>) {
    const gestureStateRef = useRef<GestureState>({
        isPinching: false,
        isSliding: false,
        initialTouchPoints: [],
        lastTouchPoints: [],
        startTime: 0
    });

    const getDistance = (point1: TouchPoint, point2: TouchPoint): number => {
        const dx = point2.x - point1.x;
        const dy = point2.y - point1.y;
        return Math.sqrt(dx * dx + dy * dy);
    };

    const getCenterPoint = (point1: TouchPoint, point2: TouchPoint): TouchPoint => {
        return {
            x: (point1.x + point2.x) / 2,
            y: (point1.y + point2.y) / 2
        };
    };

    const getTouchPoints = (touches: Touch[]): TouchPoint[] => {
        return Array.from(touches).map((touch: Touch) => ({
            x: touch.clientX,
            y: touch.clientY
        }));
    };

    const isVerticalSlide = (dx: number, dy: number): boolean => {
        return Math.abs(dy) > Math.abs(dx) && Math.abs(dy) > 10;
    };

    const isHorizontalSlide = (dx: number, dy: number): boolean => {
        return Math.abs(dx) > Math.abs(dy) && Math.abs(dx) > 10;
    };

    const handleTouchStart = (event: TouchEvent): void => {
        console.log("touchstart", event);

        const touchPoints = getTouchPoints(event.touches);

        gestureStateRef.current.initialTouchPoints = [...touchPoints];
        gestureStateRef.current.lastTouchPoints = [...touchPoints];
        gestureStateRef.current.startTime = Date.now();

        console.log(touchPoints.length);

        if (touchPoints.length === 2) {
            // 双指触摸，可能是缩放
            gestureStateRef.current.isPinching = true;
            gestureStateRef.current.initialDistance = getDistance(touchPoints[0], touchPoints[1]);

            const center = getCenterPoint(touchPoints[0], touchPoints[1]);
            props.onPinch?.({
                detail: {
                    center,
                    distance: gestureStateRef.current.initialDistance
                }
            });
        }
    };

    const handleTouchMove = (event: TouchEvent): void => {
        const touchPoints = getTouchPoints(event.touches);
        console.log(touchPoints.length);

        if (touchPoints.length === 2 && gestureStateRef.current.isPinching) {
            // 双指缩放
            const currentDistance = getDistance(touchPoints[0], touchPoints[1]);
            const scale = currentDistance / (gestureStateRef.current.initialDistance || 1);
            const center = getCenterPoint(touchPoints[0], touchPoints[1]);

            props.onPinch?.({
                detail: {
                    scale,
                    center,
                    currentDistance,
                    initialDistance: gestureStateRef.current.initialDistance
                }
            });
        } else if (touchPoints.length === 1 && !gestureStateRef.current.isPinching) {
            // 单指滑动
            if (gestureStateRef.current.initialTouchPoints.length === 1) {
                const dx = touchPoints[0].x - gestureStateRef.current.initialTouchPoints[0].x;
                const dy = touchPoints[0].y - gestureStateRef.current.initialTouchPoints[0].y;
                const timeElapsed = Date.now() - gestureStateRef.current.startTime;

                if (!gestureStateRef.current.isSliding) {
                    if (isVerticalSlide(Math.abs(dx), Math.abs(dy))) {
                        gestureStateRef.current.isSliding = true;
                        props.onSlideStart?.({
                            detail: {
                                direction: dy > 0 ? 'down' : 'up',
                                startX: gestureStateRef.current.initialTouchPoints[0].x,
                                startY: gestureStateRef.current.initialTouchPoints[0].y
                            }
                        });
                    }
                }

                if (gestureStateRef.current.isSliding) {
                    const direction = dy > 0 ? 'down' : 'up';
                    const velocity = dy / (timeElapsed || 1);

                    props.onSlide?.({
                        detail: {
                            direction,
                            dx,
                            dy,
                            velocity,
                            currentX: touchPoints[0].x,
                            currentY: touchPoints[0].y
                        }
                    });
                }
            }

            gestureStateRef.current.lastTouchPoints = [...touchPoints];
        }
    }

    const handleTouchEnd = (): void => {
        if (gestureStateRef.current.isPinching) {
            gestureStateRef.current.isPinching = false;
            props.onPinchEnd?.();
        }

        if (gestureStateRef.current.isSliding) {
            gestureStateRef.current.isSliding = false;
            props.onSlideEnd?.();
        }

        // 重置状态
        gestureStateRef.current.initialTouchPoints = [];
        gestureStateRef.current.lastTouchPoints = [];
        gestureStateRef.current.initialDistance = undefined;
        gestureStateRef.current.startTime = 0;
    }

    return (
        <view className={props.className} style={props.style}
            bindtouchstart={handleTouchStart}
            bindtouchmove={handleTouchMove}
            bindtouchend={handleTouchEnd}
            bindtouchcancel={handleTouchEnd}
        >
            {props.children}
        </view>
    )
}