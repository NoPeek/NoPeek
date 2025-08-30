// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { useEffect, useRef } from "@lynx-js/react";
import AddIconSolid from "../assets/icons/icon_add_solid.png";
import { useNavigate } from "react-router";
import { Toast, type ToastHandle } from "@/components/toast";

// 9images
import Image1 from "../assets/images/1.jpg";
import Image2 from "../assets/images/2.jpg";
import Image3 from "../assets/images/3.jpg";
import Image4 from "../assets/images/4.jpg";
import Image5 from "../assets/images/5.jpg";
import Image6 from "../assets/images/6.jpg";
import Image7 from "../assets/images/7.jpg";
import Image8 from "../assets/images/8.jpg";
import Image9 from "../assets/images/9.jpg";

export default function PostPage() {
    const navigate = useNavigate();
    const toastRef = useRef<ToastHandle>(null);

    const handleCreatePost = () => {
        navigate('/sanitization');
    };

    const handleTryAIFeature = () => {
        navigate('/sanitization');
    };

    const displayImages = [Image1, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Image9].slice(0, 9);

    useEffect(() => {
        toastRef.current?.show();
    }, []);

    return (
        <view className="w-screen h-screen relative overflow-hidden flex flex-col justify-between">
            <Toast ref={toastRef} style={{
                borderRadius: "16px",
                bottom: "7.5vh"
            }} content="Please tap the Button above to try our Product!" />

            {/* 添加与Sanitization相同的背景 */}
            <view className="fade-to-right-bottom w-screen h-screen absolute top-0 left-0 opacity-50">
                <view className="bg-isometric size-full scale-150"></view>
            </view>
            
            {/* 主要内容区域 */}
            <view className=" flex flex-col items-center px-6 py-40">
                
                {/* 标题 */}
                <text className="text-4xl text-black font-extrabold text-left mb-4 bg-gradient-to-r from-blue-600 to-indigo-600 bg-clip-text leading-tight whitespace-nowrap">
                   Image Selection Page
                </text>
                <text className="text-1xl text-gray-700 text-center font-medium mb-8 px-4">
                   The pictures we post often contain a lot of <text className="font-bold">sensitive information.</text>
                </text>
                
                {/* 3x3 图片网格 */}
                <view className="w-full aspect-square mb-10">
                <view className="grid grid-cols-3 grid-rows-3 gap-3 w-full h-full">
                    {displayImages.map((image, index) => (
                    <view
                        key={index}
                        className="relative overflow-hidden rounded-2xl shadow-md bg-gray-100"
                    >
                        <image
                        className="w-full h-full object-cover transform transition-transform duration-300 hover:scale-110"
                        src={image}
                        mode="aspectFill"
                        />
                    </view>
                    ))}
                </view>
                </view>

                {/* 占位区域 - 给按钮更多向下空间 */}
                <view className="flex-grow"></view>
                {/* AI功能尝试按钮 - 向下移动更多距离 */}
                <view className="relative p-2 rounded-full overflow-hidden bg-primary shadow-primary flex items-center justify-center -mb-[10vh]">
                    <view className="absolute size-full bg-isometric" style={{ opacity: 0.2 }}></view>
                    <text 
                        className="text-xl font-black rounded-full text-white px-10 py-3 text-center"
                        bindtap={handleTryAIFeature}
                    >
                        ✨ Try Tiktok AI Privacy New Feature!
                    </text>
                </view>
            </view>
            
            {/* 底部小横条  bg-[#1f2937] */}
            <view className="w-full h-24 bg-[#FFFFF] bg-opacity-80 backdrop-blur-sm flex items-center justify-center">
                <view 
                    className="w-16 h-16 flex items-center justify-center cursor-pointer transform transition-transform active:scale-95"
                    bindtap={handleCreatePost}
                >
                    <image 
                        auto-size
                        className="h-12 -mt-9" 
                        // src={AddIconOutline} 
                        src={AddIconSolid} 
                    />
                </view>
            </view>
        </view>
    );
}
