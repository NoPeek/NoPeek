// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


export const calculateMasonryImageHeight = (
    width: number, height: number,
    masonryPadding: number, masonryMainAxisGap: number,
    masonrySpanNumber: number
): number => (
    (
        SystemInfo.pixelWidth / SystemInfo.pixelRatio - 
        masonryPadding * 2 - masonryMainAxisGap
    ) / masonrySpanNumber / (width / height)
)