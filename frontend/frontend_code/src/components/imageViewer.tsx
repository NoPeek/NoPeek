// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState, useRef, useEffect } from "@lynx-js/react";
import { useNavigate } from "react-router";
import clsx from "clsx";

import FloatingButton from "./floatingButton.jsx";
import { Toast, type ToastHandle } from "./toast.js";
import ConfirmDialog from "./confirmDialog.jsx";
import type { Picture, PictureAlteredInfo } from "@/types/picture.js";
import InfoPanel from "./infoPanel.jsx";
import { useTiktokDemo } from "@/utils/useTiktokDemo.jsx";

import XMarkIcon from "../assets/icons/icon_xmark_outline.png";
import CompareIcon from "../assets/icons/icon_compare_outline.png";
import InfoIcon from "../assets/icons/icon_info_outline.png";
import InfoIconSolid from "../assets/icons/icon_info_solid.png";
import TiktokIcon from "../assets/icons/icon_tiktok_solid.png";
import DownloadIcon from "../assets/icons/icon_download_outline.png";
import TrashIcon from "../assets/icons/icon_trash_outline.png";

export default function ImageViewer({ 
    image, overlayImage, alteredInfo, onClose, onPhotoRemoved
}: { 
    image: Picture, overlayImage?: Picture, alteredInfo: PictureAlteredInfo,
    onClose?: () => void,
    onPhotoRemoved?: (image: Picture) => void
}) {
    const isTiktokDemo = useTiktokDemo();
    const navigate = useNavigate();

    const toastRef = useRef<ToastHandle>(null);
    const [isInfoPanelVisible, setIsInfoPanelVisible] = useState(false);

    const screenWidth = SystemInfo.pixelWidth / SystemInfo.pixelRatio;
    const screenHeight = SystemInfo.pixelHeight / SystemInfo.pixelRatio;
    
    //#region Gesture Detection
    const [currentTranslation, setCurrentTranslation] = useState({ x: 0, y: 0 });
    const [translation, setTranslation] = useState({ x: 0, y: 0 });

    const [currentScale, setCurrentScale] = useState(1);
    const [scaleFactor, setScaleFactor] = useState(1);

    const handlePan = ({ detail: { translationX, translationY, state } }: { detail: { translationX: number; translationY: number; state: 1|2|3 } }) => {
        switch(state) {
            case 1:
                setCurrentTranslation({ x: translation.x, y: translation.y });
                break;
            case 2:
                const imageWidth = screenWidth;
                const imageHeight = screenWidth * (image.height / image.width);
                const newTranslation = { 
                    x: Math.max(-imageWidth / 2 * (scaleFactor - 1), Math.min(currentTranslation.x + translationX, imageWidth / 2 * (scaleFactor - 1))),
                    y: Math.max(-imageHeight / 2 * (scaleFactor - 1), Math.min(currentTranslation.y + translationY, imageHeight / 2 * (scaleFactor - 1)))
                };
                setTranslation(newTranslation);
                break;
        }
    };

    const handlePinch = ({ detail: { scale, state } }: { detail: { scale: number; state: number } }) => {
        switch(state) {
            case 1:
                setCurrentScale(scaleFactor);
                break;
            case 2:
                const newScaleFactor = Math.max(1, Math.min(currentScale * scale, 3));
                if (scale <= 1) {
                    const imageWidth = screenWidth;
                    const imageHeight = screenWidth * (image.height / image.width);
                    const newTranslation = { 
                        x: Math.max(-imageWidth / 2 * (newScaleFactor - 1), Math.min(translation.x, imageWidth / 2 * (newScaleFactor - 1))),
                        y: Math.max(-imageHeight / 2 * (newScaleFactor - 1), Math.min(translation.y, imageHeight / 2 * (newScaleFactor - 1)))
                    };
                    setTranslation(newTranslation);
                }
                setScaleFactor(newScaleFactor);
                break;
        }
    };
    //#endregion Gesture Detection

    //#region Save Photo
    const handleSavePhotoSuccess = () => {
        console.log("Photo saved successfully");
        toastRef.current?.show();
        setTimeout(() => toastRef.current?.hide(), 4000);
    };

    const handleSavePhotoFailed = (error: any) => {
        console.error("Photo failed to save", error);
    };

    const savePhotoToAlbum = () => {
        lynx.createSelectorQuery()
            .select("#image-viewer-photo-saver")
            .invoke({
                method: "saveImage",
                params: {
                    image: image.src,
                    fileName: "ProtectedPhoto_" + image.id + ".jpg",
                },
                success(res) {
                    console.log("Photo saved:", res);
                },
                fail(err) {
                    console.error("Failed to save photo:", err);
                }
            })
            .exec();
    };
    //#endregion Save Photo

    //#region Delete Photo
    const [isPendingDeletion, setIsPendingDeletion] = useState(false);

    const handlePendingDeletion = () => {
        setIsPendingDeletion(true);
    };

    const handleDeletionConfirmed = () => {
        onPhotoRemoved?.(image);
        onClose?.();
    };

    const handleDeletionCancelled = () => {
        setIsPendingDeletion(false);
    };
    //#endregion Delete Photo

    //#region Overlay
    const [isOverlayVisible, setIsOverlayVisible] = useState(false);

    const handleShowOverlay = () => {
        setIsOverlayVisible(true);
    };

    const handleHideOverlay = () => {
        setIsOverlayVisible(false);
    };
    //#endregion Overlay

    //#region InfoPanel
    const [infoPanelHeight, setInfoPanelHeight] = useState(screenHeight * 0.3);

    useEffect(() => {
        lynx.createSelectorQuery()
            .select("#image-viewer-info-panel")
            .invoke({
                method: "boundingClientRect",
                success: (res) => (res && res.height) && setInfoPanelHeight(res.height + 30),
                fail: () => setInfoPanelHeight(screenHeight * 0.3)
            })
            .exec();
    }, []);

    const handleInfoPanelToggle = () => {
        setIsInfoPanelVisible(!isInfoPanelVisible);
    };
    //#endregion InfoPanel

    const handleNavigateToTiktok = () => {
        navigate("/postproceed", {
            state: {
                images: [
                    { id: image.id, src: image.src }
                ]
            }
        });
    };

    return (
        <view className={clsx(
            "fixed left-0 top-0 w-screen h-screen grid grid-cols-1 items-center bg-black"
        )}>
            <view className={clsx(
                "absolute left-0 top-0 size-full -z-10 bg-white",
                "transition-opacity duration-500 ease-in-out",
                isInfoPanelVisible ? "opacity-100" : "opacity-0"
            )}>
                <view className="size-full bg-isometric scale-150" style={{ opacity: 0.15 }}></view>
            </view>
            <Toast ref={toastRef} content="Photo saved to album" />
            <photo-saver id="image-viewer-photo-saver" bindsuccess={handleSavePhotoSuccess} bindfailed={handleSavePhotoFailed} />
            <view className="absolute left-0 top-0 size-full z-10">
                <gesture-recognizer
                    width={SystemInfo.pixelWidth / SystemInfo.pixelRatio}
                    height={SystemInfo.pixelHeight / SystemInfo.pixelRatio}
                    enablePinch enablePan
                    bindpan={handlePan}
                    bindpinch={handlePinch}
                />
            </view>
            <view className="relative" style={{
                width: screenWidth + "px",
                height: screenWidth * (image.height / image.width) + "px",
                transform: `translate(${translation.x}px,${translation.y}px) scale(${scaleFactor})`,
                transformOrigin: `center center`
            }}>
                <image className="size-full" src={image.src} />
                {
                    (isOverlayVisible && overlayImage && overlayImage.src) && (
                        <image src={overlayImage.src} 
                            className="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2" 
                            style={{
                                width: screenWidth + "px",
                                height: screenWidth * (overlayImage.height / overlayImage.width) + "px",
                            }} 
                        />
                    )
                }
            </view>

            {
                isPendingDeletion && <ConfirmDialog
                    onConfirm={handleDeletionConfirmed}
                    onCancel={handleDeletionCancelled}
                >
                    <view className="flex flex-col items-center gap-1 px-4">
                        <text className="font-bold text-xl">Delete this protected photo?</text>
                        <text className="text-lg tracking-wide">Confirm?</text>
                    </view>
                </ConfirmDialog>
            }

            {/* Close Button */}
            <FloatingButton className="absolute z-50 right-[6.25vw] top-[12.5vh] p-[7px] shadow-primary" icon={XMarkIcon} onClick={onClose} />
            <view className={clsx(
                "absolute z-50 bottom-[10vh] w-[87.5vw] left-[6.25vw]",
                "flex justify-between items-center",
                "transition-transform duration-500 ease-in-out"
            )} style={{
                transform: isInfoPanelVisible ? `translateY(${-infoPanelHeight + screenHeight * 0.1 - 20}px)` : "translateY(0)"
            }}>
                <view className={clsx(
                    "flex gap-3 rounded-full px-2 py-[0.75px] bg-neutral-200 shadow-primary"
                )}>
                    {/* Info Button */}
                    <FloatingButton className="p-[5px]" icon={isInfoPanelVisible ? InfoIconSolid : InfoIcon} onClick={handleInfoPanelToggle} />
                    {/* Comparison Button */}
                    { overlayImage && <FloatingButton className="p-2" icon={CompareIcon} onTouchStart={handleShowOverlay} onTouchEnd={handleHideOverlay} /> }
                    {/* Download Button */}
                    <FloatingButton className="p-2" icon={DownloadIcon} onClick={savePhotoToAlbum} />
                    {/* TikTok Button */}
                    { isTiktokDemo && <FloatingButton className="p-[6px]" icon={TiktokIcon} onClick={handleNavigateToTiktok} /> }
                </view>
                {/* Delete Button */}
                <FloatingButton className="p-2 shadow-primary" icon={TrashIcon} onClick={handlePendingDeletion} />
            </view>

            {/* Info Panel */}
            <view className="relative w-full self-end transition-all duration-500 ease-in-out" style={{
                height: isInfoPanelVisible ? `${infoPanelHeight}px` : "0"
            }}>
                <InfoPanel id="image-viewer-info-panel" className="w-full px-[6.25vw] py-4" alteredInfo={alteredInfo} />
            </view>
        </view>
    )
}