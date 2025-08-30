// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useEffect } from "@lynx-js/react";
import type { Picture } from "@/types/picture.js";
import { calculateMasonryImageHeight } from "@/utils/calculateMasonryImageHeight.js";

const ImageWrapper = (props: { 
    src: string; index?: number;
    onClick?: () => void;
}) => {
    const delay = (props.index ?? 0) * 75;
    const [isLoaded, setIsLoaded] = useState(false);

    useEffect(() => {
        setTimeout(() => setIsLoaded(true), delay);
    }, []);

    return (
        <view className="size-full rounded-md overflow-hidden anim-fade-in" style={{
            visibility: isLoaded ? "visible" : "hidden",            
        }}>
            <image className="size-full" src={props.src} bindtap={props.onClick} />
        </view>
    )
};

export default function Masonry(props: {
    className?: string,
    xPadding?: number,
    images: Array<Picture>,
    onImageClick?: (imageId: string) => void
}) {
    const columnCount = 2;
    const mainAxisGap = 10;

    return (
        <view className={`gallery-wrapper ${props.className}`}>
            <list className="size-full pb-[40vh]" style={{
                //@ts-ignore
                listMainAxisGap: mainAxisGap + "px", listCrossAxisGap: "10px"
            }} list-type="waterfall" column-count={columnCount} scroll-orientation="vertical">
                {
                    props.images.map((image, index) => (
                        <list-item style={{
                            height: calculateMasonryImageHeight(
                                image.width, image.height,
                                props.xPadding ?? 0, mainAxisGap, columnCount
                            ) + "px"
                        }} item-key={image.id} key={image.id}>
                            <ImageWrapper src={image.src} index={index} onClick={() => props.onImageClick?.(image.id)} />
                        </list-item>
                    ))
                }
            </list>
        </view>
    )
}