// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { MemoryRouter, Routes, Route } from "react-router";
import PageLayout from "@/pages/pageLayout.js";
import { standaloneRoutes, tabBarRoutes } from "./routes.js";
import ProceedPage from "@/pages/postproceed.js"
import BrowsePage from "@/pages/browseposts.js"

export default function Router() {
    return (
        <MemoryRouter>
            <Routes>
                {/* Entry页面独立渲染，不继承pageLayout */}
                {/* <Route path="/" element={routes.find(r => r.index)?.element()} /> */}
                {/* 重定向根路径到首页 */}
                {/* <Route path="/" element={<EntryPage />} /> */}
                <Route path="/" element={<BrowsePage />} />
                <Route path="/postproceed" element={<ProceedPage />} />
                <Route path="/browseposts" element={<BrowsePage />} />
                
                {/* 独立页面（无布局） */}
                {standaloneRoutes.map(({ path, element: Element }) => (
                    <Route 
                        key={path} 
                        path={path} 
                        element={<Element />} 
                    />
                ))}
                {/* 其他页面继承pageLayout */}
                <Route element={<PageLayout />}>
                    {tabBarRoutes.map(({ path, index, element: Element }) => (
                        <Route 
                            key={path} 
                            index={index}
                            path={index ? undefined : path}
                            element={<Element />} 
                        />
                    ))}
                </Route>
            </Routes>
        </MemoryRouter>
    );
}
