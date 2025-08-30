// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { createContext, useContext } from "@lynx-js/react";

const TiktokDemoContext = createContext<boolean>(false);

export const useTiktokDemo = () => {
    return useContext(TiktokDemoContext);
};

export const TiktokDemoProvider = TiktokDemoContext.Provider;