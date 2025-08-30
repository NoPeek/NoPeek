// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useRef, useEffect, useState, type PropsWithChildren } from "@lynx-js/react";
import { useNavigate } from "react-router";
import type { CSSProperties } from "@lynx-js/types";
import clsx from "clsx";

import { useProcessImage } from "@/utils/useProcessImage.jsx";
import { removeExif } from "@/utils/exifErasor.js";
import ImageComparator from "@/components/imageComparator.jsx";
import FloatingButton from "@/components/floatingButton.jsx";
import { Slider, SliderItem, type SliderHandle } from "@/components/slider.jsx";
import SelectorGroup from "@/components/selectorGroup.jsx";
import { emptyPictureAlteredInfo, type PictureAlteredInfo } from "@/types/picture.js";
import { generatePictureID, parseFilenameID } from "@/utils/generatePictureID.js";
import { 
    fetchFaceDetectedCoordinates, type FaceDetectedCoordinates,
    fetchFaceMaskedImage, fetchInfoRemovedImage
} from "@/api/index.js";

import XMarkIcon from "@/assets/icons/icon_xmark_outline.png";
import ProcessIcon from "@/assets/icons/icon_process_outline.png";
import ShieldSolidWhiteIcon from "@/assets/icons/icon_shield_solid_white.png";
import ShieldSolidIcon from "@/assets/icons/icon_shield_solid.png";
import LoaderIcon from "@/assets/icons/icon_loader.png";
import CheckAnim from "@/assets/anim/icon_shield_anim_400.gif";
const checkAnimAspectRatio = 492 / 484;
const checkAnimDuration = 1920; // ms

//#region Components
const SliderTitle = (props: { title: string }) => (
    <text className="absolute top-[17.5vh] text-3xl font-black">{props.title}</text>
);

const Button = (props: PropsWithChildren<{ onClick?: () => void, className?: string, style?: CSSProperties }>) => {
    const [isPressed, setIsPressed] = useState(false);
    const handleButtonTouchStart = () => setIsPressed(true);
    const handleButtonTouchEnd = () => setIsPressed(false);
    return (
        // idk why the transform-origin seems to be changed when using `scale`
        <view className={clsx(
                "size-full flex justify-center items-center py-1 rounded-lg transition-opacity m-auto",
                props.className,
                isPressed ? "opacity-90" : "opacity-100",
            )} style={{
                transform: isPressed ? "scale(0.95)" : "",
                ...props.style 
            }} 
            bindtap={props.onClick}
            bindtouchstart={handleButtonTouchStart} 
            bindtouchend={handleButtonTouchEnd}
            bindtouchcancel={handleButtonTouchEnd}
        >
            {props.children}
        </view>
    )
};
//#endregion Components

//#region Erase Exif
const EraseExifSlider = (props: {
    originalImageSrc: string,
    alteredInfo: PictureAlteredInfo,
    onAlteredInfoChanged: (info: PictureAlteredInfo) => void,
    slideToNext: () => void,
    slideByOffset: (offset: number) => void
}) => {
    const { image, setImage } = useProcessImage();
    if (!image) return <view>Unknown Error Occurred</view>;

    const screenHeight = SystemInfo.pixelHeight / SystemInfo.pixelRatio;
    const imageWidth = SystemInfo.pixelWidth / SystemInfo.pixelRatio * 0.85;
    const imageHeight = imageWidth * (image.height / image.width);

    const exifSensitiveContentTitle = [
        "GPS Coordinates",
        "Date Taken",
        "Device Model"
    ];

    const [animatingStage, setAnimatingStage] = useState<0|1|2|3|4>(0);
    const centralImageOffsetFactor = -0.08;

    const handleSlideToNext = () => {
        if (animatingStage != 4) return;
        props.slideToNext();
    };

    const handleSkipFaceDetection = () => {
        if (animatingStage != 4) return;
        props.slideByOffset(2);
    };

    useEffect(() => {
        setTimeout(() => {
            // Moving image downwards
            setAnimatingStage(1);

            const res = removeExif(image.src);
            setImage({ ...image, src: res });
            props.onAlteredInfoChanged({ ...props.alteredInfo, exifErased: true });

            setTimeout(() => {
                // Showing background cards
                setAnimatingStage(2);
                setTimeout(() => {
                    // Moving background cards out
                    setAnimatingStage(3);
                    setTimeout(() => {
                        // Reset image
                        setAnimatingStage(4);
                    }, 1200);
                }, 1700);
            }, 1500);
        }, 1000);
    }, []);

    return (
        <view className="relative size-full flex flex-col justify-center items-center">
            <SliderTitle title="Erasing EXIF Data" />
            <view className="relative w-[85vw] -mb-[5vh]">
                {
                    animatingStage != 0 && exifSensitiveContentTitle.map((title, index) => (
                        <view key={index} className={clsx(
                            "absolute w-full h-1/2 flex justify-center", 
                            "bg-gray-900 border-2 border-black rounded-2xl py-2 transition-transform"
                        )} style={{
                            zIndex: -(index + 1),
                            transitionDuration: `500ms`,
                            transitionDelay: `${(index) * 300}ms`,
                            transform: (
                                animatingStage == 2 ? `
                                    translateY(${(index + 1) * -3.5}vh) 
                                    scale(${1 - (index + 1) * 0.05})
                                ` : (
                                    (animatingStage == 3 || animatingStage == 4) ? `
                                        translate(-100vw, ${(index + 1) * -3.5}vh) 
                                        scale(${1 - (index + 1) * 0.05})
                                    ` : ""
                                )
                            )
                        }}>
                            <text className="text-white font-black">{title}</text>
                        </view>
                    ))
                }
                <image className={clsx(
                    "rounded-2xl transition-transform ease-in-out",
                    "anim-zoom-in"
                )} style={{ 
                    width: imageWidth + "px", height: imageHeight + "px",
                    transform: (animatingStage == 0 || animatingStage == 4) ? `translateY(${centralImageOffsetFactor * screenHeight}px)` : "",
                    transitionDuration: "1000ms"
                }} src={props.originalImageSrc} />
            </view>
            <view className={clsx(
                "w-[85%] flex flex-col justify-center items-center gap-2",
                "absolute bottom-[12.5vh] mx-auto px-4 py-6 bg-white rounded-2xl shadow-primary",
                animatingStage == 4 ? "animate-fade-up animate-duration-700 visible" : "invisible"
            )}>
                <text className="text-xl font-bold">EXIF Data Removed!</text>
                <text className="text-lg text-center leading-tight">Continue with <text className="text-xl font-bold text-center text-primary">Face Detection</text> for additional privacy protection?</text>
                <view className="w-full flex justify-between mt-3">
                    <Button className="w-[40%] border-4 border-primary" onClick={handleSkipFaceDetection}>
                        <text className="text-primary font-bold text-center leading-tight">Skip Face Detection</text>
                    </Button>
                    <Button className="w-[55%] gap-2 bg-primary px-6" onClick={handleSlideToNext}>
                        <image className="min-w-8 h-8" src={ShieldSolidWhiteIcon} />
                        <text className="text-white font-bold text-center leading-tight">Start Face Detection</text>
                    </Button>
                </view>
            </view>
        </view>
    )
};
//#endregion Erase Exif

//#region Face Detect
const DetectFacesSlider = (props: {
    isFakeRendering: boolean,
    alteredInfo: PictureAlteredInfo,
    onAlteredInfoChanged: (info: PictureAlteredInfo) => void,
    slideToNext: () => void,
    navigateToGallery: () => void
}) => {
    const { image, setImage } = useProcessImage();
    if (!image) return <view>Unknown Error Occurred</view>;

    const originalImageRef = useRef(image.src);
    const screenHeight = SystemInfo.pixelHeight / SystemInfo.pixelRatio;
    const imageWidth = SystemInfo.pixelWidth / SystemInfo.pixelRatio;
    const imageHeight = imageWidth * (image.height / image.width);

    // TODO: handle error
    const [stage, setStage] = useState<"idling"|"detecting"|"detected"|"masking"|"beforedone"|"done"|"error">("idling");
    const centralImageOffsetFactor = -0.07;
    const [faceCoordinates, setFaceCoordinates] = useState<FaceDetectedCoordinates["coordinates"]>([]);

    const handleSlideToNext = (forced?: boolean) => {
        if (stage != "done" && !forced) return;
        props.slideToNext();
    };

    const handleSkipMasking = () => {
        if (stage != "detected") return;
        handleSlideToNext(true);
    };

    const handleMaskingStart = (type: "blur"|"sticker"|"cartoon") => {
        if (stage != "detected") return;
        setStage("masking");
        fetchFaceMaskedImage(image.id, type, image.src).then((res) => {
            if (res) {
                setImage({ ...image, src: res });
                setStage("beforedone");
                props.onAlteredInfoChanged({ 
                    ...props.alteredInfo,
                    faceBlurred: type == "blur",
                    faceCartooned: type == "cartoon",
                    faceStickered: type == "sticker"
                });
                setTimeout(() => {
                    setStage("done");
                }, 500);
            } else {
                console.error("Failed to fetch masked image");
                setStage("beforedone");
                setTimeout(() => {
                    setStage("done");
                }, 500);
            }
        });
    };

    const handleSkipInfoDetection = () => {
        if (stage != "done") return;
        props.navigateToGallery();
    };

    const faceProcessConfig: { caption: string, action: () => void }[] = [{
        caption: "Blur",
        action: () => handleMaskingStart("blur")
    }, {
        caption: "Sticker",
        action: () => handleMaskingStart("sticker")
    }, {
        caption: "Cartoon",
        action: () => handleMaskingStart("cartoon")
    }];

    useEffect(() => {
        console.log("isFakeRendering:", props.isFakeRendering);
        if (!props.isFakeRendering) {
            setStage("detecting");
            fetchFaceDetectedCoordinates(image.id, image.src).then((res) => {
                setStage("detected");
                setTimeout(() => setFaceCoordinates(res.coordinates.map((coord) => ({
                    x: coord.x * imageWidth,
                    y: coord.y * imageHeight,
                    width: coord.width * imageWidth,
                    height: coord.height * imageHeight
                }))), 500);
            });
        }
    }, []);

    return (
        <view className="relative size-full flex flex-col justify-center items-center">
            <SliderTitle title="Face Detection" />
            {
                (stage == "detecting" || stage == "masking") && <view className="text-sky-100 absolute size-full flex flex-col gap-6 justify-center items-center z-40 animate-fade animate-ease-in-out">
                    <view className="absolute size-full bg-white opacity-65"></view>
                    <image className="size-20 animate-spin" src={LoaderIcon} />
                    <text className="text-primary text-2xl font-black">{ stage == "detecting" ? "Detecting Faces..." : "Masking Faces..." }</text>
                </view>
            }
            {
                (stage != "done") ? <view className="relative size-fit">
                    <image className="" src={originalImageRef.current} style={{
                        width: imageWidth + "px",
                        height: imageHeight + "px",
                        borderRadius: (["idling", "detecting", "beforedone"].includes(stage)) ? "16px" : "0px",
                        transitionProperty: "all",
                        transitionTimingFunction: "ease-in-out",
                        transitionDuration: stage == "beforedone" ? "500ms" : "300ms",
                        transform: (
                            (stage == "idling" || stage == "detecting") ? "scale(0.85)" : (
                                stage == "beforedone" ? `translateY(${centralImageOffsetFactor * screenHeight}px) scale(0.85)` : "scale(1)"
                            )
                        )
                    }} />
                    {/* Recognition Bounding */}
                    {
                        stage == "detected" && <view className="absolute size-full">
                            {
                                faceCoordinates.map((coord, index) => (
                                    <view key={index} className={clsx(
                                        "absolute border-2 border-white rounded-sm", 
                                        "shadow-[0_0_0_4px_#0369a1]"
                                    )} style={{
                                        left: coord.x + "px",
                                        top: coord.y + "px",
                                        width: (coord.width + 3) + "px",
                                        height: (coord.height + 3) + "px"
                                    }}>
                                        <view className="absolute -left-[8px] -top-[8px] size-3 bg-white"></view>
                                        <image className="absolute -left-[14px] -top-[14px] size-6" src={ShieldSolidIcon} />
                                    </view>
                                ))
                            }
                        </view>
                    }
                </view> :
                <ImageComparator className="rounded-2xl overflow-hidden" style={{
                    width: (imageWidth * 0.85) + "px",
                    height: (imageHeight * 0.85) + "px",
                    transform: `translateY(${centralImageOffsetFactor * screenHeight}px)`
                }} leftImage={originalImageRef.current} rightImage={image.src} 
                leftText="BEFORE" rightText="AFTER" />
            }
            {
                // Stage 2: Detected, show button group
                (stage == "detected" || stage == "masking") && <view className={clsx(
                    "flex flex-col gap-3 absolute bottom-[12.5vh] mx-auto w-[85%]",
                    "px-6 py-5 bg-white rounded-2xl shadow-primary",
                    "transition-opacity duration-300 ease-in-out",
                    stage == "masking" && "opacity-0"
                )}>
                    <view className="grid grid-cols-2 gap-3">
                        {
                            faceProcessConfig.map((item, index) => (
                                <Button key={index} className={clsx(
                                    "size-full bg-primary rounded-lg gap-2"
                                )} onClick={item.action}>
                                    <image className="min-w-7 h-7" src={ShieldSolidWhiteIcon} />
                                    <text className="w-fit text-white text-lg py-2 font-bold text-center leading-[1] tracking-wide">{item.caption}</text>
                                </Button>
                            ))
                        }
                        <Button className="border-4 border-primary rounded-lg" onClick={handleSkipMasking}>
                            <text className="text-primary text-lg py-1 font-bold text-center leading-[1]">Skip</text>
                        </Button>
                    </view>
                    <view><text className="text-sm">Select how you want faces to be masked</text></view>
                </view>
            }
            {/* Stage 5: Masked */}
            <view className={clsx(
                "w-[85%] flex flex-col justify-center items-center absolute bottom-[12.5vh] mx-auto gap-4",
                "px-6 py-4 bg-white rounded-2xl shadow-primary",
                stage == "done" ? "animate-fade-up animate-duration-700 visible" : "invisible"
            )}>
                <text className="text-xl font-bold">Face Masked!</text>
                <text className="text-xl text-center leading-tight">Continue with <text className="text-xl font-bold text-center text-primary">Information Masking</text> for additional privacy protection?</text>
                <view className="w-full flex justify-between mt-3">
                    <Button className="w-[40%] border-4 border-primary" onClick={handleSkipInfoDetection}>
                        <text className="text-primary font-bold text-center leading-tight">Skip Info Detection</text>
                    </Button>
                    <Button className="w-[55%] bg-primary gap-2 px-6" onClick={handleSlideToNext}>
                        <image className="min-w-8 h-8" src={ShieldSolidWhiteIcon} />
                        <text className="text-white font-bold text-center leading-tight">Start Info Detection</text>
                    </Button>
                </view>
            </view>
        </view>
    )
};
//#endregion Face Detect

//#region Detect Sensitive Content
const DetectSensitiveContentSlider = (props: {
    originalImageSrc: string,
    alteredInfo: PictureAlteredInfo,
    onAlteredInfoChanged: (info: PictureAlteredInfo) => void,
    navigateToGallery: () => void;
}) => {
    const { image, setImage } = useProcessImage();
    if (!image) return <view>Unknown Error Occurred</view>;

    const originalImageRef = useRef(image.src);
    const screenWidth = SystemInfo.pixelWidth / SystemInfo.pixelRatio;
    const screenHeight = SystemInfo.pixelHeight / SystemInfo.pixelRatio;
    const imageWidth = screenWidth;
    const imageHeight = imageWidth * (image.height / image.width);

    // TODO: handle error
    const [stage, setStage] = useState<"idling"|"masking"|"beforedone"|"done"|"error">("idling");
    const centralImageOffsetFactor = -0.08;
    const [shouldHideCheckAnim, setShouldHideCheckAnim] = useState(false);

    type InfoProcess = "geolocation" | "id_card" | "document_file" | "license_plate";
    const infoProcessOptions: { label: string, value: InfoProcess, disabled?: boolean }[] = [
        { label: "Mask License Plate", value: "license_plate" },
        { label: "Mask Document File", value: "document_file" },
        { label: "Mask ID Card", value: "id_card", disabled: true },
        { label: "Mask Geolocation", value: "geolocation", disabled: true },
    ];
    const defaultSelected = infoProcessOptions.filter(v => !v.disabled).map(v => v.value);
    const [selectedInfoProcess, setSelectedInfoProcess] = useState<InfoProcess[]>(defaultSelected);

    const handleMaskingStart = () => {
        if (stage != "idling") return;

        if (selectedInfoProcess.length == 0) handleNavigateToGallery(true);

        setStage("masking");
        fetchInfoRemovedImage(
            image.id, 
            selectedInfoProcess.filter(o => o !== "id_card" && o !== "geolocation"), 
            image.src
        ).then((res) => {
            if (res) {
                setImage({ ...image, src: res });
                setStage("beforedone");
                props.onAlteredInfoChanged({ 
                    ...props.alteredInfo,
                    licensePlateMasked: selectedInfoProcess.includes("license_plate"),
                    documentFileMasked: selectedInfoProcess.includes("document_file"),
                    idCardMasked: selectedInfoProcess.includes("id_card"),
                    geolocationMasked: selectedInfoProcess.includes("geolocation")
                });
                setTimeout(() => {
                    setStage("done");
                    // Hide the Check GIF
                    setTimeout(() => {
                        setShouldHideCheckAnim(true);
                    }, checkAnimDuration + 1750);
                }, 500);
            } else {
                console.error("Failed to fetch masked image");
                setStage("beforedone");
                setTimeout(() => {
                    setStage("done");
                    // Hide the Check GIF
                    setTimeout(() => {
                        setShouldHideCheckAnim(true);
                    }, checkAnimDuration + 1750);
                }, 500);
            }
        });
    };

    const handleNavigateToGallery = (forced?: boolean) => {
        if (stage != "done" && !forced) return;
        props.navigateToGallery();
    };

    const handleSkipMasking = () => handleNavigateToGallery(true);

    return (
        <view className="relative size-full flex flex-col justify-center items-center">
            <SliderTitle title="Information Masking" />
            {
                stage == "masking" && <view className="text-sky-100 absolute size-full flex flex-col gap-6 justify-center items-center z-40 animate-fade">
                    <view className="absolute size-full bg-white opacity-65"></view>
                    <image className="size-20 animate-spin" src={LoaderIcon} />
                    <text className="text-primary text-2xl font-black">{ "Masking Info..." }</text>
                </view>
            }
            <view className="relative" style={{
                width: (imageWidth * 0.85) + "px",
                height: (imageHeight * 0.85) + "px"
            }}>
                {
                    (stage != "done") && <image className="absolute size-full" src={originalImageRef.current} style={{
                        borderRadius: (stage == "idling" || stage == "beforedone") ? "16px" : "0px",
                        transitionProperty: "all",
                        transitionTimingFunction: "ease-in-out",
                        transitionDuration: "300ms",
                        transform: (
                            stage == "idling" ? 
                                `translateY(${centralImageOffsetFactor * screenHeight}px)` : 
                                `translateY(0)`
                        )
                    }} />
                }
                {
                    (stage == "beforedone" || stage == "done") && <ImageComparator className={clsx(
                        "absolute size-full rounded-2xl overflow-hidden",
                        stage == "beforedone" && "animate-fade animate-duration-150"
                    )} leftImage={props.originalImageSrc} rightImage={image.src} 
                    leftText="BEFORE" rightText="AFTER" />
                }
            </view>
            {
                stage == "idling" && <view className="absolute bottom-[10.5vh] w-[85%] bg-white rounded-2xl shadow-primary px-4 py-4 flex flex-col gap-4">
                    <SelectorGroup
                        className="w-full"
                        caption="Select Information to Mask"
                        defaultValues={defaultSelected}
                        options={infoProcessOptions}
                        values={selectedInfoProcess}
                        onChange={setSelectedInfoProcess}
                    />
                    <view className="w-full flex justify-between">
                        <Button className="w-[40%] border-4 border-primary" onClick={handleSkipMasking}>
                            <text className="text-primary font-bold text-xl text-center leading-tight">Skip</text>
                        </Button>
                        <Button className="w-[55%] bg-primary" style={{ borderRadius: "8px", padding: "8px 0" }} onClick={handleMaskingStart}>
                            <text className="text-white font-bold text-xl text-center leading-tight">Mask Info</text>
                        </Button>
                    </view>
                </view>
            }
            {
                stage == "done" && <view className={clsx(
                    "absolute bottom-[12.5vh] w-[85%] bg-white rounded-2xl shadow-primary px-4 py-7", 
                    "flex flex-col items-center gap-2 overflow-hidden",
                    stage == "done" ? "animate-fade-up animate-duration-700 visible" : "invisible"
                )}>
                    <view className="absolute size-full bg-isometric" style={{ opacity: 0.3, transform: "scale(1.5)" }}></view>
                    <text className="text-xl font-bold">Your Photo is Protected!</text>
                    <image style={{ 
                        width: `${screenWidth * 0.3}px`,
                        height: shouldHideCheckAnim ? "0px" : `${screenWidth * 0.3 / checkAnimAspectRatio}px`,
                        transitionProperty: "height",
                        transitionTimingFunction: "ease-in-out",
                        transitionDuration: "500ms"
                    }} src={CheckAnim} loop-count={1} />
                    <view className="w-full flex justify-between">
                        <Button className="w-[75%] bg-primary gap-2 px-6" style={{ padding: "8px 0" }} onClick={handleNavigateToGallery}>
                            <image className="min-w-7 min-h-7" src={ShieldSolidWhiteIcon} />
                            <text className="text-white font-bold text-center leading-tight">View in Gallery</text>
                        </Button>
                    </view>
                </view>
            }
        </view>
    )
};
//#endregion Detect Sensitive Content

export default function ProcessPage(props: {
    originalImageSrc: string,
    onVisibilityChange?: (isVisible: boolean) => void;
}) {
    const { image } = useProcessImage();
    const sliderRef = useRef<SliderHandle>(null);
    const [isProcessing, setIsProcessing] = useState(false);
    const alteredInfoRef = useRef<PictureAlteredInfo>(emptyPictureAlteredInfo);
    const navigate = useNavigate();

    const handleSlideToNext = () => {
        sliderRef.current?.slideToNext();
    };

    const handleSlideByOffset = (offset: number) => {
        sliderRef.current?.slideBy(offset);
    };

    const handleCloseProcessPage = () => {
        !isProcessing && props.onVisibilityChange?.(false);
    };

    const handleNavigateToGallery = () => {
        const pictureID = generatePictureID(parseFilenameID(image?.id ?? ""), alteredInfoRef.current);
        console.log("File to save:", pictureID);
        !!image && NativeModules.NativeImageFileManager.saveImageFromBase64(image.src, pictureID, ({ result, message }) => {
            if (result) {
                console.log("Image saved successfully:", message);
            } else {
                console.error("Failed to save image:", message);
            }
        });
        navigate("/gallery", { state: { transition: true } });
        handleCloseProcessPage();
    };

    return (
        <view className={clsx(
            "fixed w-screen h-screen bg-white top-0 left-0 z-30",
            "animate-fade animate-duration-150"
        )}>
            <view className="absolute size-full bg-isometric-5 scale-150 -z-10" />
            <FloatingButton className={clsx(
                    "absolute z-50 right-[6.25vw] top-[12.5vh] p-[7px] shadow-lg animate-duration-[3000ms]",
                    isProcessing && "animate-spin"
                )} bgColorClass="bg-slate-100" 
                icon={isProcessing ? ProcessIcon : XMarkIcon} disabled={isProcessing}
                onClick={handleCloseProcessPage} 
            />
            <Slider 
                initialIndex={0}
                ref={sliderRef}
                className="w-screen h-screen"
                items={[EraseExifSlider, DetectFacesSlider, DetectSensitiveContentSlider].map((Slide, index) => (
                    (isFakeRendering) => (
                        <SliderItem key={index} className="size-full">
                            <Slide 
                                isFakeRendering={isFakeRendering}
                                alteredInfo={alteredInfoRef.current}
                                onAlteredInfoChanged={(info) => alteredInfoRef.current = info}
                                originalImageSrc={props.originalImageSrc}
                                slideByOffset={handleSlideByOffset}
                                slideToNext={handleSlideToNext} 
                                navigateToGallery={handleNavigateToGallery} 
                            />
                        </SliderItem>
                    )
                ))}
            />
        </view>
    )
}