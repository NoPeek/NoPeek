// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import Piexif from "piexifjs";

export const removeExif = (base64: string): string => {
    try {
        return Piexif.remove(base64);
    } catch (error) {
        console.error("Error removing EXIF data:", error);
        return base64;
    }
};