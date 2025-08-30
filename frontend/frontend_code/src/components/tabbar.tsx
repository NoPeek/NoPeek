// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import type { PropsWithChildren } from "@lynx-js/react";
import { useLocation, useNavigate } from "react-router";
import clsx from "clsx";
import routes from "@/routes/routes.js";

const TabBarButton = ({ path, title, active, children }: PropsWithChildren<{ 
    path: string, 
    title: string,
    active: boolean
}>) => {
    const navigate = useNavigate();

    return (
        <view className="flex flex-col items-center justify-center" bindtap={() => navigate(path)}>
            {children}
            <text className={clsx("text-sm font-bold mt-1", { "text-primary": active })}>{title}</text>
        </view>
    );
};

export default function TabBar() {
    const location = useLocation();
    const currentRoute = routes.find(route => route.path === location.pathname);

    return (
        <view className={clsx(
            "fixed w-[80vw] left-[10vw] bottom-[5.5vh] bg-white",
            "flex flex-row justify-around items-center",
            "py-3 rounded-full shadow-primary",
        )}>
            {
                routes.map(route => (
                    route.showInTabBar && <TabBarButton
                        key={route.path}
                        path={route.path}
                        title={route.title}
                        active={location.pathname === route.path}
                    >
                        <image className="size-7" src={location.pathname === route.path ? route.activeIcon : route.normalIcon} />
                    </TabBarButton>
                ))
            }
        </view>
    )
}