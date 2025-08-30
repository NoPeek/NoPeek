// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

/** @type {import('tailwindcss').Config} */
const colors = require("tailwindcss/colors");

export default {
    content: ["./src/**/*.{html,js,ts,jsx,tsx}"],
    presets: [
        require("@lynx-js/tailwind-preset")
    ],
    theme: {
        extend: {
            colors: {
                primary: {
                    DEFAULT: colors.sky[700]
                }
            },
            boxShadow: {
                "primary": "0 5px 25px rgba(0,0,0,0.1),0 5px 10px rgba(0,0,0,0.1)"
            } 
        }
    },
    plugins: [
        require("tailwindcss-animated")
    ]
};