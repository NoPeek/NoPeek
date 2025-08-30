// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useRef, useEffect } from "@lynx-js/react";
import type { TouchEvent, NodesRef, CSSProperties } from "@lynx-js/types";

export default function ImageComparator(props: {
    className?: string,
    style?: CSSProperties,
    leftText?: string,
    rightText?: string,
    leftImage: string,
    rightImage: string
}) {
    const containerRef = useRef<NodesRef>(null);
    const [containerWidth, setContainerWidth] = useState<number>(0);
    const [containerLeft, setContainerLeft] = useState<number>(0);

    const handlerWidth = 48;
    const handlerRef = useRef<NodesRef>(null);
    const touchStartXRef = useRef<number>(0);
    const currentOffsetRef = useRef<number>(0);
    const [handlerOffset, setHandlerOffset] = useState<number>(0);

    const handleTouchStart = (event: TouchEvent) => {
        touchStartXRef.current = event.touches[0].clientX;
    };

    const handleTouchMove = (event: TouchEvent) => {
        const offset = event.touches[0].clientX - containerLeft - handlerWidth / 2;
        setHandlerOffset(Math.max(-handlerWidth / 2 + 8 + 4, Math.min(offset, (containerWidth - handlerWidth / 2 - 8 - 4))));
    };

    useEffect(() => {
        containerRef.current?.invoke({
            method: "boundingClientRect",
            success: (res) => {
                const { width, left } = res;
                setContainerLeft(left);
                setContainerWidth(width);
                currentOffsetRef.current = width / 2;
                // Initialize handler offset
                setHandlerOffset(width / 2 - handlerWidth / 2);
            }
        }).exec();
    }, []);

    return (
        <view ref={containerRef} className={`image-comparator relative ${props.className}`} style={props.style}>
            <view className="absolute size-full overflow-hidden">
                <image className="size-full" src={props.leftImage} />
                { props.leftText && <text className="absolute bottom-2 left-4 text-base font-black text-white">{props.leftText}</text> }
            </view>
            <view ref={handlerRef} className="absolute z-10 bg-primary opacity-90 rounded-full top-1/2 left-0" style={{
                width: `${handlerWidth}px`, height: `${handlerWidth}px`,
                transform: `translate(${handlerOffset}px, -50%)`,
            }} bindtouchstart={handleTouchStart} bindtouchmove={handleTouchMove} />
            <view className="absolute w-1 h-full z-[9] bg-primary opacity-50 top-0 left-0" style={{
                transform: `translateX(${handlerOffset + handlerWidth / 2 - 2}px)`
            }}></view>
            <view className="absolute size-full overflow-hidden" style={{
                clipPath: `inset(0 0 0 ${handlerOffset + handlerWidth / 2}px)`
            }}>
                <image className="size-full" src={props.rightImage} />
                { props.rightText && <text className="absolute bottom-2 right-4 text-base font-black text-white">{props.rightText}</text> }
            </view>
        </view>
    )
}