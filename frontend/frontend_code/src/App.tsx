// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { TiktokDemoProvider } from "./utils/useTiktokDemo.jsx";
import Router from "./routes/router.js"
import "./App.css";

const isTiktokDemo = true;

export function App(props: {
    onRender?: () => void
}) {
    props.onRender?.()

    return (
        <TiktokDemoProvider value={isTiktokDemo}>
            <Router />
        </TiktokDemoProvider>
    )
}