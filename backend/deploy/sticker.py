// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import os, glob, random, json
import cv2
import numpy as np
import argparse
from typing import List, Dict, Tuple, Optional


def _denorm_xyxy(b, W: int, H: int) -> Tuple[int, int, int, int]:
    x1 = int(round(float(b[0]) * W)); y1 = int(round(float(b[1]) * H))
    x2 = int(round(float(b[2]) * W)); y2 = int(round(float(b[3]) * H))
    return x1, y1, x2, y2

def _assert_in_bounds(x1, y1, x2, y2, W, H, what: str):
    if x1 < 0 or y1 < 0 or x2 > W or y2 > H:
        raise ValueError(f"{what} exceeds image bounds {W}x{H}: {(x1, y1, x2, y2)}")
    if x2 <= x1 or y2 <= y1:
        raise ValueError(f"Invalid {what} geometry: {(x1, y1, x2, y2)}")

def _expand_bbox(x1, y1, x2, y2, W, H, pct=0.15, what="expanded bbox"):
    bw, bh = x2 - x1, y2 - y1
    dx = int(round(bw * pct)); dy = int(round(bh * pct))
    ex1, ey1, ex2, ey2 = x1 - dx, y1 - dy, x2 + dx, y2 + dy
    _assert_in_bounds(ex1, ey1, ex2, ey2, W, H, what)
    return ex1, ey1, ex2, ey2

def _load_detections(json_path: str) -> List[Dict]:
    """
    Load detections from a JSON file.
    Expected format: a list of dicts, each with 'bbox_xyxy' (normalized xyxy).
    Other keys like 'confidence' or 'attributes' are ignored by this script.
    """
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("Detections JSON must be a list of objects.")
    out = []
    for d in data:
        if isinstance(d, dict) and "bbox_xyxy" in d:
            bb = d["bbox_xyxy"]
            if isinstance(bb, (list, tuple)) and len(bb) == 4:
                # keep attributes if present (e.g., gender)
                attrs = d.get("attributes", {}) if isinstance(d.get("attributes", {}), dict) else {}
                out.append({"bbox_xyxy": bb, "attributes": attrs})
    return out


def place_face_stickers(
    bgr: np.ndarray,
    dets: List[Dict],
    stickers_dir: str = "stickers",
    *,
    expand_pct: float = 0.15,     # grow bbox a bit so sticker fully covers face
    fit_mode: str = "cover",      # "cover" fits the smaller dimension, may crop; "contain" fits inside
    max_aspect_stretch: float = 1.3
) -> np.ndarray:
    """
    Overlay gendered transparent PNG stickers on detected faces.

    Inputs:
      - bgr: HxWx3 BGR uint8 image
      - dets: list of dicts with key "bbox_xyxy" in normalized [0..1] xyxy and
              optional gender string at d["attributes"].get("gender", "") in {"male","female"}

    Behavior:
      - Picks a random sticker per face from stickers_dir/vecteezy_male_*.png or vecteezy_female_*.png
      - Expands the bbox by expand_pct
      - Resizes sticker to (roughly) cover the expanded bbox while preserving transparency
      - Alpha-blends onto the image, safely handling edges/out-of-bounds

    Returns:
      - New image with stickers applied (uint8 BGR)
    """
    if not dets:
        return bgr

    H, W = bgr.shape[:2]
    out = bgr.copy()

    # Cache sticker file lists by gender
    male_paths   = sorted(glob.glob(os.path.join(stickers_dir, "vecteezy_male_*.png")))
    female_paths = sorted(glob.glob(os.path.join(stickers_dir, "vecteezy_female_*.png")))
    all_paths = (male_paths or []) + (female_paths or [])

    def _pick_path(gender: str) -> Optional[str]:
        g = (gender or "").strip().lower()
        if g == "male" and male_paths:
            return random.choice(male_paths)
        if g == "female" and female_paths:
            return random.choice(female_paths)
        # Fallback: pick from whichever exists
        return random.choice(all_paths) if all_paths else None

    def _overlay_bgra(dst: np.ndarray, sticker_bgra: np.ndarray, x: int, y: int) -> None:
        """
        Alpha-blend a BGRA sticker onto dst (BGR) with top-left at (x,y).
        Clips to image bounds automatically.
        """
        sh, sw = sticker_bgra.shape[:2]
        if sh <= 0 or sw <= 0:
            return

        # Compute ROI in destination (clip to bounds)
        x0, y0 = max(0, x), max(0, y)
        x1, y1 = min(W, x + sw), min(H, y + sh)
        if x0 >= x1 or y0 >= y1:
            return

        # Corresponding area in the sticker
        sx0 = x0 - x
        sy0 = y0 - y
        sx1 = sx0 + (x1 - x0)
        sy1 = sy0 + (y1 - y0)

        roi_dst = dst[y0:y1, x0:x1]
        roi_src = sticker_bgra[sy0:sy1, sx0:sx1]

        if roi_src.shape[2] == 4:
            src_rgb = roi_src[..., :3].astype(np.float32)
            alpha   = (roi_src[..., 3:4].astype(np.float32)) / 255.0  # HxWx1
            dst_rgb = roi_dst.astype(np.float32)

            out_rgb = alpha * src_rgb + (1.0 - alpha) * dst_rgb
            roi_dst[:] = np.clip(out_rgb, 0, 255).astype(np.uint8)
        else:
            # No alpha channel—just paste
            roi_dst[:] = roi_src

    for d in dets:
        # Resolve gender and pick a sticker
        gender = ""
        if isinstance(d, dict) and isinstance(d.get("attributes", None), dict):
            gender = d["attributes"].get("gender", "") or ""
        sticker_path = _pick_path(gender)
        if not sticker_path:
            # No stickers available; skip gracefully
            continue

        # Load sticker with alpha
        sticker = cv2.imread(sticker_path, cv2.IMREAD_UNCHANGED)
        if sticker is None:
            continue
        # Ensure BGRA
        if sticker.ndim == 2:
            sticker = cv2.cvtColor(sticker, cv2.COLOR_GRAY2BGRA)
        elif sticker.shape[2] == 3:
            # No alpha; synthesize fully opaque
            a = np.full(sticker.shape[:2] + (1,), 255, dtype=np.uint8)
            sticker = np.concatenate([sticker, a], axis=2)

        # Compute (expanded) bbox in pixel space
        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        try:
            x1e, y1e, x2e, y2e = _expand_bbox(x1, y1, x2, y2, W, H, pct=expand_pct, what="sticker bbox")
        except Exception:
            # If expansion fails due to border, fall back to raw bbox clipped
            x1e, y1e, x2e, y2e = max(0, x1), max(0, y1), min(W, x2), min(H, y2)

        bw, bh = max(1, x2e - x1e), max(1, y2e - y1e)

        # Compute sticker size: keep aspect; choose scale depending on fit_mode
        sh0, sw0 = sticker.shape[:2]
        src_aspect = sw0 / max(1e-6, sh0)
        box_aspect = bw / max(1e-6, bh)

        if fit_mode == "contain":
            # Fit entirely within the box
            scale = min(bw / sw0, bh / sh0)
        else:  # "cover"
            scale = max(bw / sw0, bh / sh0)

        # Moderate aspect stretch if desired
        if fit_mode == "cover" and (box_aspect / src_aspect > max_aspect_stretch or src_aspect / box_aspect > max_aspect_stretch):
            scale = max(bw / sw0, bh / sh0)

        new_w = max(1, int(round(sw0 * scale)))
        new_h = max(1, int(round(sh0 * scale)))
        sticker_resized = cv2.resize(sticker, (new_w, new_h), interpolation=cv2.INTER_AREA)

        # Center the sticker over the expanded bbox
        ox = x1e + (bw - new_w) // 2
        oy = y1e + (bh - new_h) // 2

        _overlay_bgra(out, sticker_resized, ox, oy)

    return out


def place_plate_stickers(bgr: np.ndarray, dets: List[Dict], sticker_path="stickers/vecteezy_plate.png", expand_pct=0.15) -> np.ndarray:
    """
    Overlay a fixed transparent PNG sticker on each detected plate.
    """
    H, W = bgr.shape[:2]
    out = bgr.copy()

    sticker = cv2.imread(sticker_path, cv2.IMREAD_UNCHANGED)
    if sticker is None:
        print(f"Plate sticker not found: {sticker_path}")
        return out

    # Ensure BGRA
    if sticker.ndim == 2:
        sticker = cv2.cvtColor(sticker, cv2.COLOR_GRAY2BGRA)
    elif sticker.shape[2] == 3:
        a = 255 * np.ones(sticker.shape[:2] + (1,), dtype=np.uint8)
        sticker = np.concatenate([sticker, a], axis=2)

    def _overlay_bgra(dst, sticker_bgra, x, y):
        sh, sw = sticker_bgra.shape[:2]
        if sh <= 0 or sw <= 0:
            return
        x0, y0 = max(0, x), max(0, y)
        x1, y1 = min(W, x + sw), min(H, y + sh)
        if x0 >= x1 or y0 >= y1:
            return
        sx0, sy0 = x0 - x, y0 - y
        sx1, sy1 = sx0 + (x1 - x0), sy0 + (y1 - y0)
        roi_dst = dst[y0:y1, x0:x1]
        roi_src = sticker_bgra[sy0:sy1, sx0:sx1]

        if roi_src.shape[2] == 4:
            src_rgb = roi_src[..., :3].astype(np.float32)
            alpha   = (roi_src[..., 3:4].astype(np.float32)) / 255.0
            dst_rgb = roi_dst.astype(np.float32)
            roi_dst[:] = np.clip(alpha * src_rgb + (1.0 - alpha) * dst_rgb, 0, 255).astype(np.uint8)
        else:
            roi_dst[:] = roi_src

    for d in dets:
        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        # Expand a bit
        try:
            x1, y1, x2, y2 = _expand_bbox(x1, y1, x2, y2, W, H, pct=expand_pct, what="plate bbox")
        except Exception:
            x1, y1, x2, y2 = max(0, x1), max(0, y1), min(W, x2), min(H, y2)

        bw, bh = max(1, x2 - x1), max(1, y2 - y1)

        # Resize sticker to bbox size
        sticker_resized = cv2.resize(sticker, (bw, bh), interpolation=cv2.INTER_AREA)

        # Overlay aligned to bbox top-left
        _overlay_bgra(out, sticker_resized, x1, y1)

    return out


def main():
    ap = argparse.ArgumentParser(description="Place stickers on faces OR plates using precomputed detections from JSON.")
    ap.add_argument("-i", "--input", required=True, help="Input image path")
    ap.add_argument("-j", "--json", required=True, help="Path to detections JSON (list of {bbox_xyxy:[x1,y1,x2,y2], ...})")
    ap.add_argument("-o", "--output", default=None, help="Output image path")
    ap.add_argument("-t", "--target", choices=["face", "plate"], default="face")
    args = ap.parse_args()

    bgr = cv2.imread(args.input)
    if bgr is None:
        raise FileNotFoundError(args.input)

    dets = _load_detections(args.json)

    if args.target == "face":
        # Slightly bigger (as requested earlier): 0.25 expansion
        out = place_face_stickers(bgr, dets, stickers_dir="stickers", expand_pct=0.25)
        suffix = "_face_sticker.jpg"
        if not dets:
            print("No face boxes in JSON — output will equal input.")
    else:
        out = place_plate_stickers(bgr, dets, sticker_path="stickers/vecteezy_plate.png", expand_pct=0.15)
        suffix = "_plate_sticker.jpg"
        if not dets:
            print("No plate boxes in JSON — output will equal input.")

    out_path = args.output or os.path.splitext(args.input)[0] + suffix
    cv2.imwrite(out_path, out)
    print(f"Done. Wrote: {out_path}")


if __name__ == "__main__":
    main()