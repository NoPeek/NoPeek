// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import { useState, type PropsWithChildren } from "@lynx-js/react";

export default function ConfirmDialog({ 
    children, onConfirm, onCancel
}: PropsWithChildren<{
    onConfirm: () => void
    onCancel: () => void
}>) {
    const [isConfirmPressed, setIsConfirmPressed] = useState(false);
    const [isCancelPressed, setIsCancelPressed] = useState(false);

    return (
        <view className="fixed w-screen h-screen flex justify-center items-center z-[100] anim-zoom-in" style={{ animationDuration: "100ms" }}>
            <view className="absolute size-full bg-gray-800 opacity-25"></view>
            <view className={clsx(
                "relative w-[75%] rounded-2xl shadow-md", 
                "flex flex-col overflow-hidden"
            )}>
                <view className="absolute size-full bg-[#f2f2f7] opacity-[0.965]"></view>
                <view className="w-full min-h-[12vh] flex justify-center items-center -mb-2">
                    {children}
                </view>
                <view className="w-full h-[5vh] flex justify-center items-center border-t border-gray-300">
                    <view 
                        className={clsx(
                            "w-1/2 h-full flex justify-center items-center border-r border-gray-300",
                            { "bg-[#e5e5ea]": isCancelPressed }
                        )} bindtap={onCancel}
                        bindtouchstart={() => setIsCancelPressed(true)}
                        bindtouchend={() => setIsCancelPressed(false)} bindtouchcancel={() => setIsCancelPressed(false)}
                    >
                        <text className="text-[#ff383b] text-xl tracking-wide">Cancel</text>
                    </view>
                    <view 
                        className={clsx(
                            "w-1/2 h-full flex justify-center items-center",
                            { "bg-[#e5e5ea]": isConfirmPressed }
                        )} bindtap={onConfirm}
                        bindtouchstart={() => setIsConfirmPressed(true)}
                        bindtouchend={() => setIsConfirmPressed(false)} bindtouchcancel={() => setIsConfirmPressed(false)}
                    >
                        <text className="text-[#0088ff] font-semibold text-xl tracking-wide">Confirm</text>
                    </view>
                </view>
            </view>
        </view>
    )
}