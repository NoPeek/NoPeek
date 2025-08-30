// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

# doc_detect.py — robust document-in-photo detector with full-frame rejection + blur output
import argparse, json, math, os
from dataclasses import dataclass
from typing import List, Tuple, Optional

import cv2
import numpy as np

_HAS_PADDLE = False
try:
    from paddleocr import PaddleOCR  # pip install paddleocr
    _HAS_PADDLE = True
except Exception:
    _HAS_PADDLE = False


# =========================
# Utilities & data structs
# =========================
@dataclass
class Box:
    x1: int; y1: int; x2: int; y2: int
    conf: float
    src: str

def _clip(v, lo, hi): return max(lo, min(hi, v))

def _norm_box(b: Box, W: int, H: int) -> Tuple[float, float, float, float]:
    return (b.x1 / W, b.y1 / H, b.x2 / W, b.y2 / H)

def _area(b: Box) -> int:
    return max(0, b.x2 - b.x1) * max(0, b.y2 - b.y1)

def _iou(a: Box, b: Box) -> float:
    ix1, iy1 = max(a.x1, b.x1), max(a.y1, b.y1)
    ix2, iy2 = min(a.x2, b.x2), min(a.y2, b.y2)
    iw, ih = max(0, ix2 - ix1), max(0, iy2 - iy1)
    inter = iw * ih
    union = _area(a) + _area(b) - inter + 1e-6
    return inter / union

def _nms(boxes: List[Box], iou_thr=0.55, topk=3) -> List[Box]:
    boxes = sorted(boxes, key=lambda b: b.conf, reverse=True)
    keep: List[Box] = []
    for b in boxes:
        if all(_iou(b, k) < iou_thr for k in keep):
            keep.append(b)
        if len(keep) >= topk:
            break
    return keep

def _resize_limit(img: np.ndarray, max_side: int = 1600) -> Tuple[np.ndarray, float]:
    H, W = img.shape[:2]
    m = max(H, W)
    if m <= max_side:
        return img, 1.0
    s = max_side / float(m)
    return cv2.resize(img, (int(W*s), int(H*s)), interpolation=cv2.INTER_AREA), s

def _illum_normalize(bgr: np.ndarray) -> np.ndarray:
    b, g, r = cv2.split(bgr.astype(np.float32) + 1e-6)
    mean_b, mean_g, mean_r = np.mean(b), np.mean(g), np.mean(r)
    gm = (mean_b + mean_g + mean_r) / 3.0
    b = b * (gm / mean_b); g = g * (gm / mean_g); r = r * (gm / mean_r)
    bgr = np.clip(cv2.merge((b,g,r)), 0, 255).astype(np.uint8)
    lab = cv2.cvtColor(bgr, cv2.COLOR_BGR2LAB)
    l, a, bb = cv2.split(lab)
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8,8))
    l = clahe.apply(l)
    return cv2.cvtColor(cv2.merge((l,a,bb)), cv2.COLOR_LAB2BGR)

def _rectangularity(cnt: np.ndarray) -> float:
    area = cv2.contourArea(cnt)
    if area <= 0: return 0.0
    rect = cv2.minAreaRect(cnt); (w, h) = rect[1]
    rect_area = max(1.0, w * h)
    return float(area / rect_area)

def _aspect_prior(w: int, h: int) -> float:
    ar = w / float(h + 1e-6)
    return 1.0 - min(1.0, abs(math.log((ar + 1e-6) / 1.0)))

def _edge_map(gray: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    gx = cv2.Sobel(gray, cv2.CV_32F, 1, 0, ksize=3)
    gy = cv2.Sobel(gray, cv2.CV_32F, 0, 1, ksize=3)
    mag = cv2.magnitude(gx, gy)
    mag_u8 = np.uint8(np.clip(mag / (mag.max() + 1e-6) * 255, 0, 255))
    v = np.median(mag_u8)
    lower = int(max(0, 0.33 * v)); upper = int(min(255, 1.33 * v))
    canny = cv2.Canny(gray, lower, upper)
    mix = cv2.addWeighted(canny, 0.7, mag_u8, 0.3, 0)
    return mix, canny

# ============================
# Full-frame rejection helpers
# ============================
def _box_area_frac(x1, y1, x2, y2, W, H) -> float:
    return ((x2 - x1) * (y2 - y1)) / float(W * H + 1e-6)

def _min_margin_ratio(x1, y1, x2, y2, W, H) -> float:
    margins = [x1 / W, y1 / H, (W - 1 - x2) / W, (H - 1 - y2) / H]
    return float(max(0.0, min(margins)))

def _touching_borders(x1, y1, x2, y2, W, H, eps_px: int) -> int:
    hits = 0
    if x1 <= eps_px: hits += 1
    if y1 <= eps_px: hits += 1
    if x2 >= W - 1 - eps_px: hits += 1
    if y2 >= H - 1 - eps_px: hits += 1
    return hits

def _reject_fullframe_like(x1, y1, x2, y2, W, H, *,
                           area_cap=0.85,
                           min_margin_cap=0.02,
                           need_not_touch_two_borders=True) -> bool:
    if x2 <= x1 or y2 <= y1:
        return True
    area_frac = _box_area_frac(x1, y1, x2, y2, W, H)
    if area_frac >= area_cap:
        return True
    if _min_margin_ratio(x1, y1, x2, y2, W, H) < min_margin_cap:
        if need_not_touch_two_borders:
            eps = max(2, int(0.01 * max(W, H)))
            if _touching_borders(x1, y1, x2, y2, W, H, eps) >= 2:
                return True
    return False


# =========================================
# Proposal A: contour-driven quad candidates
# =========================================
def _proposals_contour(small: np.ndarray, full_W: int, full_H: int, inv_scale: float) -> List[Box]:
    Hs, Ws = small.shape[:2]
    gray = cv2.cvtColor(small, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (5,5), 0)
    edge_mix, _ = _edge_map(gray)

    cnts, _ = cv2.findContours(edge_mix, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    img_area = Hs * Ws
    out: List[Box] = []
    for c in cnts:
        if cv2.arcLength(c, True) < 0.12 * (Hs + Ws):
            continue
        hull = cv2.convexHull(c)
        if len(hull) < 4:
            continue
        rectness = _rectangularity(hull)
        if rectness < 0.70:
            continue

        rect = cv2.minAreaRect(hull)
        box_pts = cv2.boxPoints(rect).astype(int)
        x1, y1 = np.min(box_pts[:,0]), np.min(box_pts[:,1])
        x2, y2 = np.max(box_pts[:,0]), np.max(box_pts[:,1])
        if (x2-x1)*(y2-y1) < 0.07 * img_area:
            continue

        X1 = _clip(int(round(x1 * inv_scale)), 0, full_W-1)
        Y1 = _clip(int(round(y1 * inv_scale)), 0, full_H-1)
        X2 = _clip(int(round(x2 * inv_scale)), 0, full_W-1)
        Y2 = _clip(int(round(y2 * inv_scale)), 0, full_H-1)

        if _reject_fullframe_like(X1, Y1, X2, Y2, full_W, full_H):
            continue

        ar_score = _aspect_prior(X2-X1, Y2-Y1)
        size_score = min(1.0, ((X2-X1)*(Y2-Y1))/(0.9*full_W*full_H))
        xs1, ys1 = x1, y1; xs2, ys2 = x2, y2
        stroke = float(np.mean(edge_mix[ys1:ys2, xs1:xs2]) / 255.0) if xs2>xs1 and ys2>ys1 else 0.0
        conf = 0.15 + 0.45*rectness + 0.25*stroke + 0.10*ar_score + 0.15*size_score
        out.append(Box(X1, Y1, X2, Y2, float(min(1.0, conf)), "contour"))
    return out


# =======================================================
# Proposal B: text-ish mask (morph gradients) → big box
# =======================================================
def _proposals_textish(small: np.ndarray, full_W: int, full_H: int, inv_scale: float) -> List[Box]:
    Hs, Ws = small.shape[:2]
    gray = cv2.cvtColor(small, cv2.COLOR_BGR2GRAY)
    k = max(3, int(0.01 * max(Hs, Ws)) | 1)
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (k, k))
    grad = cv2.morphologyEx(gray, cv2.MORPH_GRADIENT, kernel)
    _, th = cv2.threshold(grad, 0, 255, cv2.THRESH_OTSU)
    th = cv2.dilate(th, kernel, iterations=1)

    cnts, _ = cv2.findContours(th, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    img_area = Hs * Ws
    out: List[Box] = []
    for c in cnts:
        x, y, w, h = cv2.boundingRect(c)
        area = w*h
        if area < 0.03 * img_area:
            continue
        if area > 0.80 * img_area:
            continue

        X1 = _clip(int(round(x * inv_scale)), 0, full_W-1)
        Y1 = _clip(int(round(y * inv_scale)), 0, full_H-1)
        X2 = _clip(int(round((x+w) * inv_scale)), 0, full_W-1)
        Y2 = _clip(int(round((y+h) * inv_scale)), 0, full_H-1)

        if _reject_fullframe_like(X1, Y1, X2, Y2, full_W, full_H):
            continue

        dens = float(np.mean(th[y:y+h, x:x+w]) / 255.0)
        ar_score = _aspect_prior(w, h)
        conf = 0.25 + 0.55*dens + 0.20*ar_score
        out.append(Box(X1, Y1, X2, Y2, float(min(1.0, conf)), "textish"))
    return out


# ==================================================
# Proposal C: line-based rectangle for broken borders
# ==================================================
def _proposals_lines(small: np.ndarray, full_W: int, full_H: int, inv_scale: float) -> List[Box]:
    Hs, Ws = small.shape[:2]
    gray = cv2.cvtColor(small, cv2.COLOR_BGR2GRAY)
    gray = cv2.bilateralFilter(gray, 7, 40, 40)
    edges = cv2.Canny(gray, 50, 150)
    lines = cv2.HoughLinesP(edges, 1, np.pi/180, threshold=120,
                            minLineLength=max(20, min(Ws, Hs)//5), maxLineGap=20)
    if lines is None or len(lines) < 2:
        return []
    segs = [l[0] for l in lines]
    xs = np.r_[ [s[0] for s in segs], [s[2] for s in segs] ]
    ys = np.r_[ [s[1] for s in segs], [s[3] for s in segs] ]
    x1, y1, x2, y2 = int(np.min(xs)), int(np.min(ys)), int(np.max(xs)), int(np.max(ys))

    X1 = _clip(int(round(x1 * inv_scale)), 0, full_W-1)
    Y1 = _clip(int(round(y1 * inv_scale)), 0, full_H-1)
    X2 = _clip(int(round(x2 * inv_scale)), 0, full_W-1)
    Y2 = _clip(int(round(y2 * inv_scale)), 0, full_H-1)

    if _reject_fullframe_like(X1, Y1, X2, Y2, full_W, full_H, area_cap=0.75, min_margin_cap=0.03):
        return []

    ar_score = _aspect_prior(X2-X1, Y2-Y1)
    edge_mean = float(np.mean(edges[y1:y2, x1:x2]) / 255.0) if x2>x1 and y2>y1 else 0.0
    conf = 0.2 + 0.6*edge_mean + 0.2*ar_score
    return [Box(X1, Y1, X2, Y2, float(min(0.9, conf)), "lines")]

def _containment_ratio(inner: Box, outer: Box) -> float:
    ix1, iy1 = max(inner.x1, outer.x1), max(inner.y1, outer.y1)
    ix2, iy2 = min(inner.x2, outer.x2), min(inner.y2, outer.y2)
    iw, ih = max(0, ix2 - ix1), max(0, iy2 - iy1)
    inter = iw * ih
    denom = _area(inner) + 1e-2
    return inter / denom

def _suppress_contained_boxes(boxes: List[Box], contain_thr: float = 0.90) -> List[Box]:
    """
    Remove any box that is mostly inside a higher-confidence box.
    contain_thr = fraction of the smaller box covered by the larger one.
    """
    if not boxes: return boxes
    boxes = sorted(boxes, key=lambda b: b.conf, reverse=True)
    keep: List[Box] = []
    for b in boxes:
        drop = any(_containment_ratio(b, k) >= contain_thr for k in keep)
        if not drop:
            keep.append(b)
    return keep

def _dedup_high_iou(boxes: List[Box], iou_thr: float = 0.70) -> List[Box]:
    """
    If two boxes have IoU >= iou_thr, keep only the higher-confidence one.
    (Greedy, confidence-descending.)
    """
    if not boxes: return boxes
    boxes = sorted(boxes, key=lambda b: b.conf, reverse=True)
    keep: List[Box] = []
    for b in boxes:
        if all(_iou(b, k) < iou_thr for k in keep):
            keep.append(b)
    return keep


# ==========================================
# OPTIONAL: PaddleOCR det map → proposal box
# ==========================================
_OCR = None
def _ensure_ocr():
    global _OCR
    if not _HAS_PADDLE: return None
    if _OCR is None:
        _OCR = PaddleOCR(det_model_dir=None, use_angle_cls=False, lang='en', show_log=False)
    return _OCR

def _proposals_paddleocr(bgr: np.ndarray) -> List[Box]:
    ocr = _ensure_ocr()
    if ocr is None: return []
    H, W = bgr.shape[:2]
    res = ocr.ocr(bgr, det=True, rec=False, cls=False)
    if not res: return []
    xs, ys = [], []
    for line in res[0]:
        poly = np.array(line[0], dtype=np.float32)
        xs.extend(list(poly[:,0])); ys.extend(list(poly[:,1]))
    x1, y1 = int(max(0, min(xs))), int(max(0, min(ys)))
    x2, y2 = int(min(W-1, max(xs))), int(min(H-1, max(ys)))
    if x2 <= x1 or y2 <= y1: return []
    if _reject_fullframe_like(x1, y1, x2, y2, W, H, area_cap=0.80, min_margin_cap=0.03):
        return []
    mask = np.zeros((H, W), np.uint8)
    for line in res[0]:
        poly = np.array(line[0], dtype=np.int32).reshape(-1,1,2)
        cv2.fillPoly(mask, [poly], 255)
    cov = float(np.mean(mask[y1:y2, x1:x2]) / 255.0)
    conf = 0.5 + 0.5*cov
    return [Box(x1, y1, x2, y2, float(min(1.0, conf)), "paddleocr")]


# ========================
# Multi-scale main detect
# ========================
def detect_documents(bgr: np.ndarray, *, max_outputs: int = 3) -> List[dict]:
    if bgr is None or bgr.size == 0: return []
    bgr = _illum_normalize(bgr)

    H, W = bgr.shape[:2]
    side_targets = [960, 1280, 1600]
    props: List[Box] = []

    for tgt in side_targets:
        small, s = _resize_limit(bgr, max_side=tgt)
        inv = 1.0 / s
        props += _proposals_contour(small, W, H, inv)
        props += _proposals_textish(small, W, H, inv)
        props += _proposals_lines(small, W, H, inv)

    try:
        props += _proposals_paddleocr(bgr)
    except Exception:
        pass

    filtered: List[Box] = []
    for p in props:
        if _reject_fullframe_like(p.x1, p.y1, p.x2, p.y2, W, H):
            continue
        if (p.x2 - p.x1) < 32 or (p.y2 - p.y1) < 32:
            continue
        filtered.append(p)

    if not filtered:
        return []

    # Start with a few more candidates, then prune aggressively
    cands = sorted(filtered, key=lambda b: b.conf, reverse=True)

    # 1) Remove boxes mostly contained in stronger boxes
    cands = _suppress_contained_boxes(cands, contain_thr=0.90)

    # 2) Remove near-duplicates with very high IoU (keeps higher-conf)
    cands = _dedup_high_iou(cands, iou_thr=0.70)

    # 3) (Optional) light NMS to space them out further
    cands = _nms(cands, iou_thr=0.55, topk=max_outputs)

    # 4) Emit JSON
    return [{
        "bbox_xyxy": list(_norm_box(b, W, H)),
        "confidence": float(max(0.0, min(1.0, b.conf))),
    } for b in cands]


# ================
# Preview drawing
# ================
def _draw_preview(bgr: np.ndarray, dets: List[dict]) -> np.ndarray:
    H, W = bgr.shape[:2]
    out = bgr.copy()
    t = max(2, int(round(0.003 * max(H, W))))
    f = 0.5 + 0.5 * min(1.0, max(H, W) / 1200.0)
    font = cv2.FONT_HERSHEY_SIMPLEX
    overlay = out.copy()
    for d in dets:
        x1n, y1n, x2n, y2n = d["bbox_xyxy"]
        x1 = _clip(int(round(x1n * W)), 0, W-1); y1 = _clip(int(round(y1n * H)), 0, H-1)
        x2 = _clip(int(round(x2n * W)), 0, W-1); y2 = _clip(int(round(y2n * H)), 0, H-1)
        if x2 <= x1 or y2 <= y1: continue
        cv2.rectangle(overlay, (x1, y1), (x2, y2), (0,255,0), -1)
        cv2.rectangle(out, (x1, y1), (x2, y2), (0,140,0), t)
        conf = d.get("confidence", None)
        label = "document" if conf is None else f"document {conf:.2f}"
        (tw, th), _ = cv2.getTextSize(label, font, f, t)
        bx1, by1 = x1, max(0, y1 - th - 8)
        bx2, by2 = min(W-1, x1 + tw + 10), min(H-1, y1 + th + 8)
        cv2.rectangle(overlay, (bx1, by1), (bx2, by2), (0,255,0), -1)
        cv2.putText(out, label, (bx1+5, min(H-1, by1+th+3)), font, f, (0,0,0), t, cv2.LINE_AA)
    out = cv2.addWeighted(overlay, 0.2, out, 0.8, 0)
    return out


# =========================
# Blur all detected regions
# =========================
def _blur_documents(bgr: np.ndarray, dets: List[dict],
                    sigma: float = 25.0,
                    feather_px_ratio: float = 0.01) -> np.ndarray:
    """
    Blur all detected boxes with soft edges.
    - sigma: Gaussian sigma for blur strength (0,0) kernel with sigmaX/Y
    - feather_px_ratio: feather width relative to max(H,W)
    """
    H, W = bgr.shape[:2]
    if not dets:
        return bgr.copy()

    # 1) A globally blurred version (so overlapping boxes blend naturally)
    blurred = cv2.GaussianBlur(bgr, (0, 0), sigmaX=sigma, sigmaY=sigma)

    # 2) Build a soft mask from all boxes
    mask = np.zeros((H, W), np.uint8)
    for d in dets:
        x1n, y1n, x2n, y2n = d["bbox_xyxy"]
        x1 = _clip(int(round(x1n * W)), 0, W-1); y1 = _clip(int(round(y1n * H)), 0, H-1)
        x2 = _clip(int(round(x2n * W)), 0, W-1); y2 = _clip(int(round(y2n * H)), 0, H-1)
        if x2 <= x1 or y2 <= y1:
            continue
        cv2.rectangle(mask, (x1, y1), (x2, y2), 255, -1)

    # Feather mask edges for nicer transitions
    feather = max(3, int(round(feather_px_ratio * max(H, W))))
    if feather % 2 == 0:
        feather += 1
    soft_mask = cv2.GaussianBlur(mask, (feather, feather), 0)

    # 3) Composite: out = soft_mask * blurred + (1-soft_mask) * original
    soft = (soft_mask.astype(np.float32) / 255.0)[..., None]  # HxWx1
    out = (soft * blurred.astype(np.float32) + (1.0 - soft) * bgr.astype(np.float32)).astype(np.uint8)
    return out


# ===========
# CLI
# ===========
def _imread_any(path: str) -> Optional[np.ndarray]:
    return cv2.imread(path, cv2.IMREAD_COLOR)


# 在 main() 函数中添加 JSON 输出
def main():
    ap = argparse.ArgumentParser(description="Robust document detector → JSON + preview + blur")
    ap.add_argument("-i", "--input", required=True)
    ap.add_argument("-o", "--output", required=False, default="")
    ap.add_argument("-k", "--topk", type=int, default=3)
    ap.add_argument("-j", "--json", required=False, default="")  # 添加 JSON 输出参数
    args = ap.parse_args()

    bgr = _imread_any(args.input)
    if bgr is None:
        raise FileNotFoundError(args.input)

    dets = detect_documents(bgr, max_outputs=args.topk)

    # 如果有指定 JSON 输出路径，使用它
    if args.json:
        json_path = args.json
    else:
        in_dir = os.path.dirname(os.path.abspath(args.input))
        base = os.path.splitext(os.path.basename(args.input))[0]
        json_path = os.path.join(in_dir, base + "_doc.json")

    # 保存 JSON
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(dets, f, indent=2)

    # 其余代码保持不变...
    # 只打印 JSON 路径，以便调用者知道在哪里可以找到它
    print(json_path)


if __name__ == "__main__":
    main()