// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import { useState, forwardRef, useImperativeHandle, type PropsWithChildren, type ReactNode } from "@lynx-js/react";
import type { CSSProperties } from "@lynx-js/types";
import ShieldIconSolid from "../assets/icons/icon_shield_solid.png";
import ShieldIconOutline from "../assets/icons/icon_shield_outline.png";

const Indicator = (props: { 
    className?: string,
    length: number, 
    currentIndex: number 
}) => {
    return (
        <view className={clsx("indicator flex justify-center items-center gap-3", props.className)}>
            {Array.from({ length: props.length }, (_, index) => (
                <image 
                    key={index} 
                    src={index <= props.currentIndex ? ShieldIconSolid : ShieldIconOutline} 
                    className={clsx(
                        "indicator-dot size-8",
                        index <= props.currentIndex ? "opacity-100" : "opacity-50"
                    )} 
                />
            ))}
        </view>
    );
};

export const SliderItem = (props: PropsWithChildren<{
    className?: string,
    style?: CSSProperties
}>) => {
    return (
        <view className={clsx("slider-item", props.className)} style={props.style}>
            {props.children}
        </view>
    );
};

export interface SliderHandle {
    currentIndex: number;
    slideToNext: () => void;
    slideBy: (offset: number) => void;
};

export const Slider = forwardRef<SliderHandle, {
    initialIndex?: number,
    items: ((isFakeRendering: boolean) => ReturnType<typeof SliderItem>)[],
    hasIndicator?: boolean,
    className?: string,
    style?: CSSProperties
}>((props, ref) => {
    const animationDuration = 500 as const;
    const [pendingIndex, setPendingIndex] = useState(props.initialIndex ?? 0);
    const [currentIndex, setCurrentIndex] = useState(props.initialIndex ?? 0);
    const [slideOffset, setSlideOffset] = useState(1);

    const [shouldInitNext, setShouldInitNext] = useState(false);
    const [isAnimating, setIsAnimating] = useState(false);

    const animateSlide = (callback?: () => void) => {
        setIsAnimating(true);
        setTimeout(() => {
            setIsAnimating(false);
            callback?.();
        }, animationDuration + 50);
    };

    const slideByOffset = (offset: number) => {
        setSlideOffset(offset);
        setPendingIndex((prev) => Math.min(Math.max(prev + offset, 0), props.items.length - 1));
        setShouldInitNext(true);
        setTimeout(() => {
            animateSlide(() => {
                setCurrentIndex((prev) => Math.min(Math.max(prev + offset, 0), props.items.length - 1));
                setTimeout(() => setShouldInitNext(false), 100);
            });
        }, 100);
    };

    useImperativeHandle(ref, () => ({
        currentIndex,
        slideToNext: () => slideByOffset(1),
        slideBy: (offset: number) => slideByOffset(offset)
    }));

    return (
        <view className={clsx(
            "relative flex justify-center items-center",
            props.className
        )}>
            {
                props.hasIndicator !== false && <Indicator 
                    className="w-3/5 h-4 absolute z-40 bottom-12 left-1/2 -translate-x-1/2"
                    length={props.items.length}
                    currentIndex={pendingIndex}
                />
            }

            <view className={clsx(
                "size-full",
                isAnimating && "transition-transform -translate-x-full"
            )} style={{ transitionDuration: `${animationDuration}ms` }}>
                { props.items[currentIndex](false) }
            </view>

            {/* Next Slide */}
            { 
                (shouldInitNext && currentIndex + slideOffset < props.items.length) &&
                <view className={clsx(
                    "absolute size-full left-full",
                    isAnimating ? "transition-all -translate-x-full opacity-100" : "opacity-0"
                )} style={{ transitionDuration: `${animationDuration}ms` }}>
                    {props.items[currentIndex + slideOffset](true)}
                </view>
            }
        </view>
    )
});