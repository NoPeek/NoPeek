// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useEffect } from "@lynx-js/react";
import { useLocation } from "react-router";
import clsx from "clsx";
import Masonry from "@/components/masonry.js";
import ImageViewer from "@/components/imageViewer.js";
import { emptyPictureAlteredInfo, type PictureAlteredInfo } from "@/types/picture.js";
import type { Picture } from "@/types/picture.js";
import { parseFilenameID, isOriginalImage, parseAlteredInfoFromPictureID } from "@/utils/generatePictureID.js";
import ShieldIconSolid from "../assets/icons/icon_shield_solid.png";

interface ImageSet {
    id: string;
    maskedImage: string;
    maskedImageID: string;
    originalImage: string;
    originalImageID: string;
    alteredInfo: PictureAlteredInfo;
    width: number;
    height: number;
};

export default function GalleryPage() {
    const { state } = useLocation();

    const [imageSets, setImageSets] = useState<ImageSet[] | null>([]);
    const [activeImage, setActiveImage] = useState<ImageSet | null>(null);

    const handleImageClick = (imageId: string) => {
        if (!imageSets) return;
        const targetImage = imageSets.find(image => image.id === imageId);
        if (!targetImage) return;
        if (activeImage?.id === imageId) return;
        console.log(targetImage);
        setActiveImage(targetImage);
    };

    const handleImageViewerClose = () => setActiveImage(null);

    const handlePhotoDeletion = (image: Picture) => {
        if (!imageSets) return;
        const imageFilenameID = parseFilenameID(image.id);
        setImageSets(imageSets.filter(img => img.id !== imageFilenameID));
        setActiveImage(null);

        const targetImageSet = imageSets.find(img => img.id === imageFilenameID);
        if (!targetImageSet) return;
        const handler = ({ result, errorMessage }: { result: boolean; errorMessage?: string }) => {
            if (result) {
                console.log("Image deleted successfully:", targetImageSet.id);
            } else {
                console.error("Failed to delete image:", errorMessage);
            }
        };
        NativeModules.NativeImageFileManager.deleteImage(targetImageSet.maskedImageID, handler);
        NativeModules.NativeImageFileManager.deleteImage(targetImageSet.originalImageID, handler);
    };

    useEffect(() => {
        let imagesOnDisk: Picture[] = [];
        let imageSets: ImageSet[] = [];
        const readImageHandler = ({ 
            fileName, result, base64OrError, width, height 
        }: { 
            fileName: string,
            result: boolean, base64OrError: string, width: number, height: number 
        }) => {
            result ?
                imagesOnDisk.push({
                    id: fileName,
                    src: base64OrError,
                    width, height
                }) :
                console.error("Failed to read image:", fileName, base64OrError);
        };

        NativeModules.NativeImageFileManager.listImages(({ result, imageFileNames }) => {
            if (result) {
                console.log("Successfully listed images:", imageFileNames);
                imageFileNames.forEach((fileName) => {
                    NativeModules.NativeImageFileManager.readImageAsBase64(fileName, (res) => {
                        readImageHandler({ fileName, ...res });
                    });
                });
                console.log("Images on disk", imagesOnDisk.map(image => ({ width: image.width, height: image.height })));

                const imageIDs = imagesOnDisk.map(img => isOriginalImage(img.id) ? parseFilenameID(img.id) : null).filter(id => id != null);
                console.log("Image group IDs:", imageIDs);
                imageIDs.forEach((id) => {
                    const groupImages = imagesOnDisk.filter(img => parseFilenameID(img.id) === id);
                    if (groupImages.length > 0) {
                        const originalImage = groupImages.findIndex(img => isOriginalImage(img.id));
                        const maskedImage = groupImages.findIndex(img => !isOriginalImage(img.id));
                        if (originalImage !== -1 && maskedImage !== -1) {
                            imageSets.push({
                                id: id,
                                maskedImage: groupImages[maskedImage].src,
                                maskedImageID: groupImages[maskedImage].id,
                                originalImage: groupImages[originalImage].src,
                                originalImageID: groupImages[originalImage].id,
                                width: groupImages[originalImage].width,
                                height: groupImages[originalImage].height,
                                alteredInfo: parseAlteredInfoFromPictureID(groupImages[maskedImage].id) || emptyPictureAlteredInfo
                            });
                        }
                    }
                });

                imageSets.length > 0 ? setImageSets(imageSets) : setImageSets(null);
            } else {
                console.error("Failed to list images:", imageFileNames);
            }
        });
    }, []);

    return (
        <view className={clsx(
            "w-screen h-screen relative", 
            state?.transition && "animate-fade-left animate-delay-150 animate-ease-in-out"
        )}>
            <view className="absolute size-full bg-isometric scale-150" style={{
                opacity: 0.15
            }}></view>
            <view className={clsx(
                "min-h-screen pt-[12.5vh] px-7 flex flex-col gap-6"
            )}>
                <view className="flex flex-col gap-1">
                    <view className="flex items-center gap-2">
                        <image className="size-9" src={ShieldIconSolid} />
                        <text className="text-[2.45rem] font-black">Gallery</text>
                    </view>
                    <text className="tracking-wide text-gray-500">Photos protected</text>
                </view>
                <view className="w-full">
                    {
                        imageSets !== null ? <Masonry className="w-full h-screen" 
                            images={imageSets.map((image) => ({
                                id: image.id,
                                src: image.maskedImage,
                                width: image.width,
                                height: image.height
                            }))} 
                            xPadding={28} // that is `px-7`, `28px for each side`
                            onImageClick={handleImageClick}
                        /> :
                        <view className="w-full flex items-center justify-center">
                            <text className="text-gray-500">No images found</text>
                        </view>
                    }
                </view>
                {
                    (!!imageSets && activeImage) && <ImageViewer 
                        // TODO: change to use the correct overlay image
                        image={{ id: activeImage.id, src: activeImage.maskedImage, width: activeImage.width, height: activeImage.height }} 
                        overlayImage={{ id: activeImage.id, src: activeImage.originalImage, width: activeImage.width, height: activeImage.height }} 
                        alteredInfo={activeImage.alteredInfo}
                        onClose={handleImageViewerClose} 
                        onPhotoRemoved={handlePhotoDeletion}
                    />
                }
            </view>
        </view>
    );
}