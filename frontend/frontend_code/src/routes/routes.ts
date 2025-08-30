// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import SparklesIconOutline from "../assets/icons/icon_sparkles_outline.png";
import SparklesIconSolid from "../assets/icons/icon_sparkles_solid.png";
import ShieldIconOutline from "../assets/icons/icon_shield_outline.png";
import ShieldIconSolid from "../assets/icons/icon_shield_solid.png";

import SanitizationPage from "@/pages/sanitization.js";
import GalleryPage from "@/pages/gallery.js";
// 新增路由
import PostPage from "@/pages/post.js";
import ProceedPage from "@/pages/postproceed.js"
import BrowsePage from "@/pages/browseposts.js"

interface RouteItem {
    index?: boolean;
    showInTabBar: boolean;
    path: string;
    title: string;
    normalIcon?: string;
    activeIcon?: string;
    element: any;
    layout?:'none' | 'tabbar';
};
// 独立页面无 Tabbar 布局
export const standaloneRoutes: RouteItem[] = [
    // {
    //     // index: true,
    //     path: "/entry",
    //     title: "Entry",
    //     normalIcon: SparklesIconOutline,
    //     activeIcon: SparklesIconSolid,
    //     element: EntryPage,
    //     layout: 'none',
    // },
    {
        // index: true,
        path: "/post",
        title: "Post",
        showInTabBar: false,
        normalIcon: SparklesIconOutline,
        activeIcon: SparklesIconSolid,
        element: PostPage,
        layout: 'none',
    },
    {
        // index: true,
        path: "/postproceed",
        title: "Proceed",
        showInTabBar: false,
        normalIcon: SparklesIconOutline,
        activeIcon: SparklesIconSolid,
        element: ProceedPage,
        layout: 'none',
    },
    {
        // index: true,
        path: "/browseposts",
        title: "Browse",
        showInTabBar: false,
        normalIcon: SparklesIconOutline,
        activeIcon: SparklesIconSolid,
        element: BrowsePage,
        layout: 'none',
    }

];

export const tabBarRoutes: RouteItem[] =[

    {
        index: false,
        path: "/sanitization",
        title: "Masking",
        showInTabBar: true,
        normalIcon: SparklesIconOutline,
        activeIcon: SparklesIconSolid,
        element: SanitizationPage,
        layout: 'tabbar',
    },
    {
        index: false,
        path: "/gallery",
        title: "Gallery",
        showInTabBar: true,
        normalIcon: ShieldIconOutline,
        activeIcon: ShieldIconSolid,
        element: GalleryPage,
        layout: 'tabbar',

    }
] 

export default [...standaloneRoutes, ...tabBarRoutes] as Array<RouteItem>;