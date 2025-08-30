// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import clsx from "clsx";
import { useState, useEffect } from "@lynx-js/react";
import type { Picture } from "@/types/picture.js";
import { generateFilenameID, generatePictureID } from "@/utils/generatePictureID.js";
import ProcessPage from "./subpages/process.js";
import { ProcessImageProvider } from "@/utils/useProcessImage.jsx";
import ImageComparator from "@/components/imageComparator.js";
import ImageRight from "../assets/image_facade_masked.jpg";
import ImageLeft from "../assets/image_facade_origin.jpg";
const facadeAspectRatio = 930 / 854;

export default function SanitizationPage() {
    const [isProcessPageVisible, setIsProcessPageVisible] = useState(false);
    const [imageToProcess, setImageToProcess] = useState<Picture | null>(null);
    const [originalImageSrc, setOriginalImageSrc] = useState<string | undefined>(undefined);

    //#region Photo Picker
    const [isPickerButtonPressed, setIsPickerButtonPressed] = useState(false);

    const openPhotoPicker = () => {
        lynx.createSelectorQuery()
            .select("#photo-picker")
            .invoke({
                method: "openPicker",
                // TODO: handle what if access is denied
                fail: handlePhotoPickerError,
            })
            .exec();
    };

    const saveImageOriginalImage = (id: string, base64: string) => {
        setOriginalImageSrc(base64);
        NativeModules.NativeImageFileManager.saveImageFromBase64(base64, id, ({ result, message }) => {
            if (result) {
                console.log("Image saved successfully:", message);
            } else {
                console.error("Failed to save image:", message);
            }
        });
    };

    const handlePickerButtonTouchStart = () => setIsPickerButtonPressed(true);
    const handlePickerButtonTouchEnd = () => setIsPickerButtonPressed(false);

    const handlePhotoPickerReturn = (res: { detail: { image: string, width: number, height: number } }) => {
        // console.log("Selected image:", res?.detail?.image);
        if (!res?.detail?.image) return;
        console.log("Image dimensions:", res?.detail?.width, "x", res?.detail?.height);
        const image: Picture = {
            id: generatePictureID(generateFilenameID()), // with _origin suffix
            src: res.detail.image,
            width: res.detail.width,
            height: res.detail.height,
        };
        saveImageOriginalImage(image.id, image.src);
        setImageToProcess(image);
        setIsProcessPageVisible(true);
    };

    const handlePhotoPickerCancel = () => {
        console.log("Photo picker canceled");
    };

    const handlePhotoPickerError = (error: any) => {
        console.error("Photo picker error:", error);
    };
    //#endregion Photo Picker

    return (
        <view className="w-screen h-screen">
            {
                !isProcessPageVisible ? <view className={clsx(
                    "w-screen h-screen flex flex-col justify-center items-center gap-10", 
                    "px-7 pb-[2.5vh]",
                    "relative"
                )}>
                    <view className="fade-to-right-bottom w-screen h-screen absolute top-0 left-0 opacity-50">
                        <view className="bg-isometric size-full scale-150"></view>
                    </view>
                    <text className="text-[2.45rem] font-black">Masking</text>
                    <ImageComparator
                        style={{ 
                            width: `${SystemInfo.pixelWidth * 0.8 / SystemInfo.pixelRatio}px`, 
                            height: `${SystemInfo.pixelWidth * 0.8 / SystemInfo.pixelRatio / facadeAspectRatio}px` 
                        }}
                        className="mt-8 mb-4 rounded-2xl overflow-hidden"
                        leftText="BEFORE" leftImage={ImageLeft}
                        rightText="AFTER" rightImage={ImageRight}
                    />
                    <view className="w-full flex flex-col items-center gap-6">
                        <view className={clsx(
                            "relative p-2 rounded-full overflow-hidden bg-primary shadow-primary",
                            isPickerButtonPressed && "scale-95 opacity-90"
                        )} bindtap={openPhotoPicker} bindtouchstart={handlePickerButtonTouchStart} 
                        bindtouchend={handlePickerButtonTouchEnd} bindtouchcancel={handlePickerButtonTouchEnd}>
                            <view className="absolute size-full bg-isometric" style={{ opacity: 0.2 }}></view>
                            <text className="text-xl font-black rounded-full text-white px-8 py-2">Mask Now!</text>
                        </view>
                        <photo-picker id="photo-picker" 
                            max-width={1920} max-height={1920} quality={0.7}
                            bindchange={handlePhotoPickerReturn} 
                            binderror={handlePhotoPickerError}
                            bindcancel={handlePhotoPickerCancel} 
                        />
                        <text className="text-base px-3">Safely upload images to automatically mask sensitive information and imagery.</text>
                    </view>
                </view> :
                <ProcessImageProvider value={{ image: imageToProcess, setImage: setImageToProcess }}>
                    { originalImageSrc && <ProcessPage originalImageSrc={originalImageSrc} onVisibilityChange={setIsProcessPageVisible} /> }
                </ProcessImageProvider>
            }
        </view>
    );
}
