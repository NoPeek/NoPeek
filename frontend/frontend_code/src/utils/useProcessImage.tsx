// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { createContext, useContext } from "@lynx-js/react";
import type { Picture } from "@/types/picture.js";

export const ProcessImageContext = createContext<{
    image: Picture | null;
    setImage: (image: Picture | null) => void;
}>({ image: null, setImage: () => {} });

export const ProcessImageProvider = ProcessImageContext.Provider;
export const useProcessImage = () => useContext(ProcessImageContext);