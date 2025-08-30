// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

/// <reference types="@lynx-js/rspeedy/client" />

// Lynx custom intrinsic elements augmentation for JSX
declare global {
  namespace JSX {
    interface IntrinsicElements {
      [elemName: string]: any;
      view: any;
      text: any;
      image: any;
      scrollview: any;
      input: {
        type?: string;
        value?: string;
        bindinput?: (e: any) => void;
        placeholder?: string;
        maxlength?: number;
        style?: string;
        className?: string;
      };
      textarea: {
        value?: string;
        bindinput?: (e: any) => void;
        placeholder?: string;
        maxlength?: number;
        style?: string;
        className?: string;
      };
    }
  }
}
