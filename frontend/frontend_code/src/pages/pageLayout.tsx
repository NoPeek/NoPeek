// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useLocation, useOutlet } from "react-router";
import TabBar from "@/components/tabbar.js";
import { tabBarRoutes } from "@/routes/routes.js";

export default function PageLayout() {
    const location = useLocation();
    const outlet = useOutlet();
    
    // 判断当前页面是否需要显示TabBar
    const shouldShowTabBar = tabBarRoutes.some(route => 
        route.path === location.pathname
    );

    return (
        <view className="w-screen h-screen">
            <view className="flex-1">
                {outlet}
            </view>
            {shouldShowTabBar && <TabBar />}
        </view>
    );
}