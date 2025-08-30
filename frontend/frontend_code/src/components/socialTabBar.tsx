// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import type { PropsWithChildren } from "@lynx-js/react";
import { useLocation, useNavigate } from "react-router";
import clsx from "clsx";

const SocialTabBarButton = ({ path, title, active, children, onClick }: PropsWithChildren<{ 
    path: string, 
    title: string,
    active: boolean,
    onClick?: () => void
}>) => {
    const navigate = useNavigate();

    const handleClick = () => {
        if (onClick) {
            onClick();
        } else {
            navigate(path);
        }
    };

    return (
        <view className="flex flex-col items-center justify-center" bindtap={handleClick}>
            {children}
            <text className={clsx("text-sm font-bold mt-1", { 
                "text-white": active, 
                "text-gray-400": !active 
            })}>{title}</text>
        </view>
    );
};

export default function SocialTabBar() {
    const location = useLocation();
    const navigate = useNavigate();

    const handleCreatePost = () => {
        navigate("/post");
    };

    const handleGoHome = () => {
        navigate("/");
    };

    const isHomePage = location.pathname === "/";
    const isPostPage = location.pathname === "/post";

    return (
        <view className={clsx(
            "fixed w-[80vw] left-[10vw] bottom-[5.5vh] bg-gray-800",
            "flex flex-row justify-around items-center",
            "py-3 rounded-full shadow-lg border border-gray-700",
        )}>
            {/* 首页按钮 */}
            <SocialTabBarButton
                path="/"
                title="首页"
                active={isHomePage}
                onClick={handleGoHome}
            >
                <view className={clsx(
                    "w-7 h-7 rounded-full flex items-center justify-center",
                    isHomePage ? "bg-blue-500" : "bg-gray-600"
                )}>
                    <text className="text-white font-bold text-sm">🏠</text>
                </view>
            </SocialTabBarButton>
            
            {/* 创建帖子按钮 */}
            <SocialTabBarButton
                path="/post"
                title="发布"
                active={isPostPage}
                onClick={handleCreatePost}
            >
                <view className="w-12 h-12 bg-gradient-to-r from-blue-500 to-purple-500 rounded-full flex items-center justify-center shadow-lg">
                    <text className="text-white font-bold text-xl">+</text>
                </view>
            </SocialTabBarButton>
            
            {/* 个人中心按钮 */}
            <SocialTabBarButton
                path="/profile"
                title="我的"
                active={false}
            >
                <view className="w-7 h-7 bg-gray-600 rounded-full flex items-center justify-center">
                    <text className="text-white font-bold text-sm">👤</text>
                </view>
            </SocialTabBarButton>
        </view>
    );
}
