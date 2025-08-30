// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import { useState } from "@lynx-js/react";

export default function FloatingButton(props: { 
    className?: string;
    disabled?: boolean;
    bgColorClass?: string;
    icon: string; 
    onClick?: () => void ;
    onTouchStart?: () => void;
    onTouchEnd?: () => void;
}) {
    const [isPressed, setIsPressed] = useState(false);
    const handlePressIn = () => {
        if (props.disabled) return;
        setIsPressed(true);
        props.onTouchStart?.();
    };
    const handlePressOut = () => {
        if (props.disabled) return;
        setIsPressed(false);
        props.onTouchEnd?.();
    };

    return (
        <view className={clsx(
            "size-11 rounded-full transition-opacity duration-300",
            props.bgColorClass ?? "bg-neutral-200",
            props.disabled ? "opacity-50" : "opacity-95",
            props.className
        )} bindtap={() => !props.disabled && props.onClick?.()} 
        bindtouchstart={handlePressIn} bindtouchend={handlePressOut} bindtouchcancel={handlePressOut}>
            <image className={clsx("size-full", isPressed && "opacity-80 scale-90")} src={props.icon} />
        </view>
    );
}