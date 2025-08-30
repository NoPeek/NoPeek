// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


export type Picture = {
    id: string; // Image filename
    src: string;
    width: number;
    height: number;
};

export interface PictureAlteredInfo {
    exifErased: boolean;
    faceBlurred: boolean;
    faceCartooned: boolean;
    faceStickered: boolean;
    licensePlateMasked: boolean;
    documentFileMasked: boolean;
    idCardMasked: boolean;
    geolocationMasked: boolean;
};

export const emptyPictureAlteredInfo: PictureAlteredInfo = {
    exifErased: false,
    faceBlurred: false,
    faceCartooned: false,
    faceStickered: false,
    licensePlateMasked: false,
    documentFileMasked: false,
    idCardMasked: false,
    geolocationMasked: false,
};