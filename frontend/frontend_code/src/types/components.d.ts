// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

/// <reference types="@lynx-js/rspeedy/client" />

import * as ReactLynx from "@lynx-js/react";
import * as Lynx from "@lynx-js/types";

declare module "@lynx-js/types" {
    interface IntrinsicElements extends Lynx.IntrinsicElements {
        "photo-picker": {
            id?: string;
            "max-width"?: number;
            "max-height"?: number;
            quality?: number;
            bindchange?: (_: { detail: { image: string, width: number, height: number } }) => void;
            binderror?: (_: any) => void;
            bindcancel?: () => void;
        };
        "photo-saver": {
            id?: string;
            bindsuccess?: () => void;
            bindfailed?: (_: any) => void;
        };
        "gesture-recognizer": {
            id?: string;
            enableTap?: boolean;
            enablePan?: boolean;
            enablePinch?: boolean;
            width?: number;
            height?: number;
            bindtap?: (_: { detail: { x: number, y: number, state: Object } }) => void;
            bindlongpress?: (_: { detail: { x: number, y: number, state: Object } }) => void;
            bindpan?: (_: { detail: { x: number, y: number, translationX: number, translationY: number, state: 1|2|3 } }) => void;
            bindswipe?: (_: { detail: { direction: "up" | "down" | "left" | "right" | "unknown", state: Object } }) => void;
            bindpinch?: (_: { detail: { scale: number, centerX: number, centerY: number, state: 1|2|3 } }) => void;
        };
        input: {
            bindinput?: (e: { type: 'input'; detail: { value: string } }) => void;
            type?: string;
            value?: string | undefined;
            placeholder?: string;
            className?: string;
            [key: string]: any;
        };
        textarea: {
            bindinput?: (e: { type: 'input'; detail: { value: string } }) => void;
            value?: string | undefined;
            placeholder?: string;
            className?: string;
            ref?: any;
            [key: string]: any;
        };
    }
}