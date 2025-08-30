// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useImperativeHandle, forwardRef } from "@lynx-js/react";
import clsx from "clsx";
import { type CSSProperties } from "@lynx-js/types";

export interface ToastHandle {
    show: () => void;
    hide: () => void;
};

export interface ToastProps {
    className?: string;
    style?: CSSProperties;
    content: string;
};

export const Toast = forwardRef<{
    show: () => void;
    hide: () => void;
}, ToastProps>((
    props: ToastProps, ref
) => {
    const [isVisible, setIsVisible] = useState<-1|0|1>(-1);

    useImperativeHandle(ref, () => ({
        show: () => setIsVisible(1),
        hide: () => setIsVisible(0)
    }));

    return (
        isVisible != -1 && <view className={clsx(
            "toast fixed w-[60vw] left-[20vw] bottom-44 z-50",
            "px-4 py-2 bg-primary rounded-full shadow-xl",
            "flex justify-center items-center", 
            "animate-fade-up animate-once animate-duration-500",
            isVisible == 1 && "animate-normal",
            isVisible == 0 && "animate-reverse",
            props.className
        )} style={props.style}>
            <text className="text-white text-lg">{props.content}</text>
        </view>
    )
});