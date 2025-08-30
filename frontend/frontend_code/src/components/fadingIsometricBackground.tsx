// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useEffect } from "@lynx-js/react";
import type { CSSProperties } from "@lynx-js/types";

export default function FadingIsometricBackground(props: {
    className?: string,
    style?: CSSProperties
}) {
    const [activeBg, setActiveBg] = useState(0);
    
    useEffect(() => {
        const interval = setInterval(() => {
            setActiveBg((prev) => (prev === 1 ? 0 : 1));
        }, 2000);
        return () => clearInterval(interval);
    }, []);

    return (
        <view className={`relative ${props.className}`} style={props.style}>
            <view className="fade-to-right-bottom w-screen h-screen absolute top-0 left-0">
                <view className="bg-isometric size-full scale-150 transition-opacity duration-[3s]" style={{
                    opacity: activeBg === 0 ? 1 : 0
                }}></view>
            </view>
            <view className="fade-to-left-top w-screen h-screen absolute top-0 left-0">
                <view className="bg-isometric size-full scale-150 transition-opacity duration-[3s]" style={{
                    opacity: activeBg === 1 ? 1 : 0
                }}></view>
            </view>
        </view>
    )
}