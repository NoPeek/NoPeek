// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


declare let NativeModules: {
    NativeLocalStorageModule: {
        setStorageItem(key: string, value: string): void;
        getStorageItem(key: string, callback: (value: string) => void): void;
        clearStorage(): void;
    };

    NativeImageFileManager: {
        saveImageFromBase64(base64: string, fileName: string, callback: (res: { result: boolean, message: string }) => void): void;
        readImageAsBase64(fileName: string, callback: (res: { result: boolean, base64OrError: string, width: number, height: number }) => void): void;
        listImages(callback: (res: { result: boolean, imageFileNames: string[] }) => void): void;
        deleteImage(fileName: string, callback: (res: { result: boolean, errorMessage: string }) => void): void;
        getImageDimensionFromBase64(base64: string, callback: (res: { result: boolean, width: number, height: number }) => void): void;
    };
}