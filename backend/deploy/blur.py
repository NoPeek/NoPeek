// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import argparse, os, cv2, numpy as np, json
from typing import List, Dict, Tuple


def _odd(n: int) -> int:
    return n if n % 2 == 1 else n + 1

def _denorm_xyxy(b, W: int, H: int) -> Tuple[int, int, int, int]:
    x1 = int(round(float(b[0]) * W)); y1 = int(round(float(b[1]) * H))
    x2 = int(round(float(b[2]) * W)); y2 = int(round(float(b[3]) * H))
    return x1, y1, x2, y2

def _assert_in_bounds(x1, y1, x2, y2, W, H, what):
    if x1 < 0 or y1 < 0 or x2 > W or y2 > H:
        raise ValueError(f"{what} exceeds image bounds {W}x{H}: {(x1, y1, x2, y2)}")
    if x2 <= x1 or y2 <= y1:
        raise ValueError(f"Invalid {what} geometry: {(x1, y1, x2, y2)}")

def _expand_bbox(x1, y1, x2, y2, W, H, pct=0.18, what="expanded bbox"):
    bw, bh = x2 - x1, y2 - y1
    dx = int(round(bw * pct)); dy = int(round(bh * pct))
    ex1, ey1, ex2, ey2 = x1 - dx, y1 - dy, x2 + dx, y2 + dy
    _assert_in_bounds(ex1, ey1, ex2, ey2, W, H, what)
    return ex1, ey1, ex2, ey2

def _ellipse_mask_from_bbox(h: int, w: int, box, grow=0.08, feather=111) -> np.ndarray:
    x1, y1, x2, y2 = box
    bw, bh = x2 - x1, y2 - y1
    cx, cy = x1 + bw // 2, y1 + bh // 2
    rx, ry = int(bw * (0.5 + grow)), int(bh * (0.55 + grow))
    _assert_in_bounds(cx - rx, cy - ry, cx + rx, cy + ry, w, h, "ellipse")
    m = np.zeros((h, w), np.uint8)
    cv2.ellipse(m, (cx, cy), (rx, ry), 0, 0, 360, 255, -1)
    if feather > 0:
        k = _odd(feather)
        m = cv2.GaussianBlur(m, (k, k), 0)
    return m

def _ring_stats(gray: np.ndarray, mask: np.ndarray, ring: int = 14):
    dil = cv2.dilate(mask, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (ring*2+1, ring*2+1)))
    ring_mask = cv2.subtract(dil, mask)
    cnt = int(np.count_nonzero(ring_mask))
    if cnt == 0:
        return 0.0, 1.0
    vals = gray[ring_mask > 0].astype(np.float32)
    mu, sd = float(vals.mean()), float(vals.std() + 1e-6)
    return mu, sd

def _match_luma(src_bgr: np.ndarray, dst_bgr: np.ndarray, mask: np.ndarray) -> np.ndarray:
    """Match luminance inside mask to the ambient luminance around it (reduces ‘pasted’ look)."""
    yuv_src = cv2.cvtColor(src_bgr, cv2.COLOR_BGR2YCrCb).astype(np.float32)
    yuv_dst = cv2.cvtColor(dst_bgr, cv2.COLOR_BGR2YCrCb).astype(np.float32)

    src_y = yuv_src[..., 0]; dst_y = yuv_dst[..., 0]
    mu_amb, sd_amb = _ring_stats(src_y.astype(np.uint8), mask, ring=14)

    inside = mask > 0
    if np.count_nonzero(inside) > 0:
        mu_in = float(dst_y[inside].mean())
        sd_in = float(dst_y[inside].std() + 1e-6)
        dst_y[inside] = (dst_y[inside] - mu_in) * (sd_amb / sd_in) + mu_amb

    yuv_dst[..., 0] = np.clip(dst_y, 0, 255)
    out = cv2.cvtColor(yuv_dst.astype(np.uint8), cv2.COLOR_YCrCb2BGR)
    return out

def _two_zone_masks(union_mask: np.ndarray, halo_px: int) -> Tuple[np.ndarray, np.ndarray]:
    """Return (inner_mask, halo_mask) where halo is a soft ring outside the inner region."""
    if halo_px < 3:
        halo_px = 3
    k = _odd(halo_px * 2 + 1)
    base = cv2.GaussianBlur(union_mask, (k, k), 0)
    dil = cv2.dilate(base, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (k, k)))
    halo_soft = cv2.GaussianBlur(dil, (k, k), 0)
    halo_soft = np.maximum(halo_soft, base)
    inner = base
    halo = np.clip(halo_soft.astype(np.int16) - inner.astype(np.int16), 0, 255).astype(np.uint8)
    return inner, halo

def _gaussian(img: np.ndarray, k: int) -> np.ndarray:
    return cv2.GaussianBlur(img, (_odd(k), _odd(k)), 0, borderType=cv2.BORDER_REPLICATE)


def blur_faces(bgr: np.ndarray, dets: List[Dict]) -> np.ndarray:
    """
    Very-strong two-zone blur for faces.
    Call with: out = blur_faces(image_bgr, face_detections)
      - image_bgr: HxWx3 BGR uint8
      - face_detections: list of dicts with key "bbox_xyxy" in normalized [0..1] xyxy
    All strengths/shapes are fixed within this function.
    """
    if not dets:
        return bgr
    H, W = bgr.shape[:2]
    out = bgr.copy()

    # Build a union mask of all faces (elliptical, expanded, wide feather)
    union = np.zeros((H, W), np.uint8)
    max_min_dim = 0
    for d in dets:
        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        x1, y1, x2, y2 = _expand_bbox(x1, y1, x2, y2, W, H, pct=0.18, what="face bbox")
        max_min_dim = max(max_min_dim, min(x2 - x1, y2 - y1))
        union = np.maximum(union, _ellipse_mask_from_bbox(H, W, (x1, y1, x2, y2), grow=0.08, feather=111))

    # Fixed strengths and halo reach (derived from bbox size)
    k_inner = int(np.clip(max_min_dim * 0.65, 61, 181))  # was 0.55 → stronger inner blur
    k_halo  = int(np.clip(max_min_dim * 0.28, 29, 99))   # was 0.22 → stronger halo blur
    halo_px = int(np.clip(max_min_dim * 0.32, 32, 140))  # was 0.26 → wider halo reach

    inner_mask, halo_mask = _two_zone_masks(union, halo_px)

    # Create blurred variants (double-pass inside for extra strength)
    inner_blur = _gaussian(out, k_inner)
    inner_blur = _gaussian(inner_blur, k_inner)
    halo_blur  = _gaussian(out, k_halo)

    # Luma-match each region to ambient ring around union
    inner_matched = _match_luma(out, inner_blur, inner_mask)
    halo_matched  = _match_luma(out, halo_blur, halo_mask)

    # Blend: almost opaque inside, noticeable outside
    a_in   = (inner_mask.astype(np.float32) / 255.0)[..., None] * 1.00  # was 0.98
    a_halo = (halo_mask.astype(np.float32)  / 255.0)[..., None] * 0.60  # was 0.50

    tmp = (a_in * inner_matched + (1.0 - a_in) * out).astype(np.uint8)
    res = (a_halo * halo_matched + (1.0 - a_halo) * tmp).astype(np.uint8)
    return res



def blur_plates(bgr: np.ndarray, dets: List[Dict]) -> np.ndarray:
    """
    Strong rectangular blur over each plate bbox using triple-pass Gaussian
    + downsample/upsample pixelation. No halo/luma-match. In-place safe.
      - image_bgr: HxWx3 BGR uint8
      - plate_detections: list of dicts with key "bbox_xyxy" in normalized [0..1] xyxy
    """
    if not dets:
        return bgr
    out = bgr.copy()
    H, W = out.shape[:2]

    for d in dets:
        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        _assert_in_bounds(x1, y1, x2, y2, W, H, "plate bbox")

        bw, bh = x2 - x1, y2 - y1
        if bw <= 1 or bh <= 1:
            continue

        # Bigger kernel and wider cap for much stronger blur
        k = int(np.clip(min(bw, bh) * 0.9, 51, 201))
        k = _odd(k)  # ensure odd

        roi = out[y1:y2, x1:x2]
        if roi.size == 0:
            continue

        # --- very strong blur: triple Gaussian ---
        roi_blur = roi
        for _ in range(3):
            roi_blur = cv2.GaussianBlur(roi_blur, (k, k), 0, borderType=cv2.BORDER_REPLICATE)

        # --- stack with pixelation (downsample -> upsample) ---
        # scale divisor chosen for strong obfuscation; adapt by bbox size
        div = 6 if min(bw, bh) >= 48 else 4  # keep at least ~8 px on tiny plates
        small_w = max(1, bw // div)
        small_h = max(1, bh // div)

        roi_small = cv2.resize(roi_blur, (small_w, small_h), interpolation=cv2.INTER_AREA)
        roi_strong = cv2.resize(roi_small, (bw, bh), interpolation=cv2.INTER_LINEAR)

        out[y1:y2, x1:x2] = roi_strong

    return out


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
    # Minimal validation
    out = []
    for d in data:
        if "bbox_xyxy" not in d:
            continue
        bb = d["bbox_xyxy"]
        if not (isinstance(bb, (list, tuple)) and len(bb) == 4):
            continue
        out.append({"bbox_xyxy": bb})
    return out


def main():
    ap = argparse.ArgumentParser(description="Blur faces OR plates using precomputed detections from JSON.")
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
        out = blur_faces(bgr, dets)
        suffix = "_face_blur.jpg"
        if not dets:
            print("No face boxes in JSON — output will equal input.")
    else:
        out = blur_plates(bgr, dets)
        suffix = "_plate_blur.jpg"
        if not dets:
            print("No plate boxes in JSON — output will equal input.")

    out_path = args.output or os.path.splitext(args.input)[0] + suffix
    cv2.imwrite(out_path, out)
    print(f"Done. Wrote: {out_path}")


if __name__ == "__main__":
    main()