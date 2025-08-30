// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import ShieldIcon from "../assets/icons/icon_shield_solid_white.png";

export default function AnnotationTag({ 
    tag, content
}: { 
    tag: string;
    content: string;
}) {
    return (
        <view className={clsx(
            "w-fit flex items-center py-2 px-5 rounded-full shadow-md bg-primary"
        )}>
            <image className="size-5 mr-2" src={ShieldIcon} />
            <text className={clsx("text-white font-bold")}>{tag}</text>
            <view className="w-[1px] h-full bg-white opacity-60 mx-2"></view>
            <text className={clsx("text-white")}>{content}</text>
        </view>
    );
}