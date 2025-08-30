// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import type { CSSProperties } from "@lynx-js/types";
import AnnotationTag from "./annotationTag.js";
import type { PictureAlteredInfo } from "@/types/picture.js";

type AlteredInfoKey = keyof PictureAlteredInfo;
const alteredInfoReadable: Record<AlteredInfoKey, { 
    tag: string, content: string
}> = {
    exifErased: { tag: "EXIF", content: "erased" },
    faceBlurred: { tag: "Face", content: "blurred" },
    faceCartooned: { tag: "Face", content: "cartooned" },
    faceStickered: { tag: "Face", content: "stickered" },
    licensePlateMasked: { tag: "License Plate", content: "masked" },
    documentFileMasked: { tag: "Document", content: "masked" },
    idCardMasked: { tag: "ID Card", content: "masked" },
    geolocationMasked: { tag: "Location", content: "masked" }
};

export default function InfoPanel({ 
    alteredInfo: info,
    id, className, style
}: { 
    alteredInfo: PictureAlteredInfo,
    id?: string,
    className?: string,
    style?: CSSProperties
}) {
    return (
        <view id={id} className={clsx(
            "flex flex-wrap gap-3",
            className
        )} style={style}>
            {
                Object.entries(info).map(([key, value]) => (
                    value && <AnnotationTag key={key} 
                        tag={alteredInfoReadable[key as AlteredInfoKey].tag as string} 
                        content={alteredInfoReadable[key as AlteredInfoKey].content as string}
                    />
                ))
            }
        </view>
    )
};