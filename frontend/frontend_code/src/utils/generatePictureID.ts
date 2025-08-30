// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { type PictureAlteredInfo } from "@/types/picture.js";

// Filename ID is the main part of the picture ID
export const generateFilenameID = () => {
    let result = '';
    for (let i = 0; i < 3; i++) {
        result += Math.random().toString(36).substring(2, 15);
    }
    return result.substring(3, 27);
};

// Picture ID is its filename
export const generatePictureID = (filenameID: string, info?: PictureAlteredInfo, ext = "jpg"): string => {
    if (!info) {
        return `${filenameID}_origin.${ext}`;
    } else {
        let flags = "";
        if (info.exifErased) flags += "ee";
        if (info.faceBlurred) flags += "fb";
        if (info.faceCartooned) flags += "fc";
        if (info.faceStickered) flags += "fs";
        if (info.licensePlateMasked) flags += "lp";
        if (info.documentFileMasked) flags += "df";
        if (info.idCardMasked) flags += "ic";
        if (info.geolocationMasked) flags += "gm";
        return `${filenameID}_${flags}.${ext}`;
    }
};

export const parseFilenameID = (pictureID: string): string => {
    const match = pictureID.match(/^(.*?)(?:_(.*?))?\.(jpg|jpeg|png|gif)$/);
    return match ? match[1] : pictureID;
};

export const parseAlteredInfoFromPictureID = (pictureID: string): PictureAlteredInfo | null => {
    const match = pictureID.match(/^(.*?)(?:_(.*?))?\.(jpg|jpeg|png|gif)$/);
    if (!match) return null;

    const flags = match[2] || "";
    console.log("Parsed flags:", flags);
    if (!flags || flags == "origin") return null;
    return {
        exifErased: flags.includes("ee"),
        faceBlurred: flags.includes("fb"),
        faceCartooned: flags.includes("fc"),
        faceStickered: flags.includes("fs"),
        licensePlateMasked: flags.includes("lp"),
        documentFileMasked: flags.includes("df"),
        idCardMasked: flags.includes("ic"),
        geolocationMasked: flags.includes("gm"),
    };
};

export const isOriginalImage = (pictureID: string): boolean => {
    const match = pictureID.match(/^(.*?)(?:_(.*?))?\.(jpg|jpeg|png|gif)$/);
    return match ? match[2] === "origin" : false;
};