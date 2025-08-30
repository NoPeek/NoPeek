// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


const baseURL = "http://127.0.0.1:8000";

export interface FaceDetectedCoordinates {
    coordinates: { x: number, y: number, width: number, height: number }[];
};

interface RawFaceDetectedCoordinates {
    detections?: {
        type: string,
        bbox_xyxy: Array<number>
    }[];
    error?: string;
};

export const fetchFaceDetectedCoordinates = async (id: string, src: string): Promise<FaceDetectedCoordinates> => {
    try {
        const request = new Request(`${baseURL}/upload`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({
                filename: id,
                image_data: src
            })
        });
        const response = await fetch(request);
        if (!response.ok) throw new Error(`Response not ok: ${response.status} ${response.statusText}`);
        const data = await response.json() as RawFaceDetectedCoordinates;
        if (data.error) throw new Error(data.error);
        if (!data.detections) return { coordinates: [] };
        return {
            coordinates: data.detections.filter(({ type }) => type === "face").map(({ bbox_xyxy }) => {
                const [x1, y1, x2, y2] = bbox_xyxy;
                return {
                    x: Number(x1.toFixed(3)),
                    y: Number(y1.toFixed(3)),
                    width: Number((x2 - x1).toFixed(3)),
                    height: Number((y2 - y1).toFixed(3))
                };
            })
        };
    } catch(error) {
        console.error("Error fetching face detected coordinates:", error);
        return {
            coordinates: []
        };
    }
};

interface RawFaceMaskedImage {
    processed_image: string;
};

export const fetchFaceMaskedImage = async (id: string, type: "blur"|"sticker"|"cartoon", src: string): Promise<string> => {
    try {
        const request = new Request(`${baseURL}/process_image`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({
                filename: id,
                image_data: src,
                type
            })
        });
        const response = await fetch(request);
        if (!response.ok) throw new Error(`Response not ok: ${response.status} ${response.statusText}`);
        const data = await response.json() as RawFaceMaskedImage;
        if (
            !data.processed_image || 
            !data.processed_image.startsWith("data:image/") ||
            !data.processed_image.includes("base64,")
        ) throw new Error("Invalid processed image format");

        return data.processed_image;
    } catch (error) {
        console.error("Error fetching face masked image:", error);
        return "";
    }
};

export const fetchInfoRemovedImage = async (id: string, types: ("license_plate"|"document_file")[], src: string): Promise<string> => {
    try {
        const request = new Request(`${baseURL}/process_doc`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({
                filename: id,
                type: types,
                image_data: src
            }),
            
        });
        const response = await fetch(request);
        if (!response.ok) throw new Error(`Response not ok: ${response.status} ${response.statusText}`);
        const data = await response.json() as RawFaceMaskedImage;
        console.log(data);
        if (
            !data.processed_image || 
            !data.processed_image.startsWith("data:image/") ||
            !data.processed_image.includes("base64,")
        ) throw new Error("Invalid processed image format");

        return data.processed_image;
    } catch (error) {
        console.error("Error fetching info removed image:", error);
        return "";
    }
};
