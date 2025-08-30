// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

# detectors.py
import os, cv2
import numpy as np
from typing import Union, Tuple, List, Dict
from PIL import Image



def _to_bgr(img: Union[str, np.ndarray, Image.Image]) -> np.ndarray:
    if isinstance(img, str):
        bgr = cv2.imread(img)
        if bgr is None:
            raise FileNotFoundError(img)
        return bgr
    if isinstance(img, np.ndarray):
        if img.ndim == 2:
            return cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
        if img.ndim == 3 and img.shape[2] == 3:
            return img
        raise ValueError("ndarray must be HxWx3 (BGR/RGB) or HxW (gray).")
    if isinstance(img, Image.Image):
        return cv2.cvtColor(np.array(img.convert("RGB")), cv2.COLOR_RGB2BGR)
    raise ValueError("img must be a path, numpy.ndarray, or PIL.Image")

def _validate_box_xyxy(x1: float, y1: float, x2: float, y2: float, w: int, h: int) -> None:
    """
    Strict validation: no clipping. If any coord exceeds image bounds or is invalid, raise.
    """
    if any(map(lambda v: not np.isfinite(v), [x1, y1, x2, y2])):
        raise ValueError(f"Non-finite bbox: {(x1, y1, x2, y2)}")
    if x1 < 0 or y1 < 0 or x2 > w or y2 > h:
        raise ValueError(f"Bbox exceeds image bounds {w}x{h}: {(x1, y1, x2, y2)}")
    if x2 <= x1 or y2 <= y1:
        raise ValueError(f"Invalid bbox geometry: {(x1, y1, x2, y2)}")

def _norm_xyxy_no_clip(x1: float, y1: float, x2: float, y2: float, w: int, h: int) -> List[float]:
    _validate_box_xyxy(x1, y1, x2, y2, w, h)
    return [x1 / w, y1 / h, x2 / w, y2 / h]

def _denorm_xyxy(b: List[float], w: int, h: int) -> Tuple[int, int, int, int]:
    x1 = int(round(b[0] * w)); y1 = int(round(b[1] * h))
    x2 = int(round(b[2] * w)); y2 = int(round(b[3] * h))
    # For drawing only; assume b already validated in [0,1] by upstream checks.
    return x1, y1, x2, y2

def detect_faces(
    img: Union[str, np.ndarray, Image.Image],
    det_size: Tuple[int, int] = (640, 640),
    preview: str = ""
) -> List[Dict]:
    """
    Returns list of dicts:
      {
        "bbox_xyxy": [x1n, y1n, x2n, y2n],  # normalized [0,1]; no clipping
        "confidence": float | None,
        "attributes": {"gender": "male"|"female"}  # never 'unknown'; raises if missing
      }
    Raises:
      - RuntimeError if gender missing/undeterminable
      - ValueError if any bbox exceeds image bounds or is invalid
    """
    from insightface.app import FaceAnalysis

    bgr = _to_bgr(img)
    h, w = bgr.shape[:2]

    # CPU only
    ctx_id = -1

    app = FaceAnalysis(name="buffalo_l")
    app.prepare(ctx_id=ctx_id, det_size=det_size)
    faces = app.get(bgr) or []

    dets: List[Dict] = []
    for f in faces:
        # Use raw float coords from model to avoid hiding out-of-bounds via int casting
        x1, y1, x2, y2 = map(float, f.bbox)

        gender = "male" if int(f.gender) == 1 else "female"

        conf = float(getattr(f, "det_score", 0.0)) if hasattr(f, "det_score") else None

        dets.append({
            "bbox_xyxy": _norm_xyxy_no_clip(x1, y1, x2, y2, w, h),
            "confidence": conf,
            "attributes": {"gender": gender}
        })

    if preview:
        vis = bgr.copy()
        for i, d in enumerate(dets):
            x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], w, h)
            cv2.rectangle(vis, (x1, y1), (x2, y2), (255, 0, 0), 2)
            label = f"face{i} ({d['attributes']['gender']})"
            cv2.putText(vis, label, (x1, max(0, y1 - 6)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.65, (255, 0, 0), 2)
        os.makedirs(os.path.dirname(preview) or ".", exist_ok=True)
        cv2.imwrite(preview, vis)

    return dets

def detect_plates(
    img: Union[str, np.ndarray, Image.Image],
    weights: str = "license_plate_detector.pt",
    conf: float = 0.25,
    iou: float = 0.5,
    preview: str = ""
) -> List[Dict]:
    """
    Returns list of dicts:
      {
        "bbox_xyxy": [x1n, y1n, x2n, y2n],  # normalized [0,1]; no clipping
        "confidence": float,
        "attributes": {}
      }
    Raises:
      - ValueError if any bbox exceeds image bounds or is invalid
    """
    from ultralytics import YOLO

    bgr = _to_bgr(img)
    h, w = bgr.shape[:2]
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

    # CPU only
    model = YOLO(weights)
    results = model.predict(source=rgb, conf=conf, iou=iou, device="cpu", verbose=False)
    r = results[0]

    if r.boxes is None:
        return []

    # Use float coords directly for validation
    boxes = r.boxes.xyxy.cpu().numpy().astype(float)
    scores = r.boxes.conf.cpu().numpy().astype(float)

    dets: List[Dict] = []
    for (x1, y1, x2, y2), score in zip(boxes, scores):
        dets.append({
            "bbox_xyxy": _norm_xyxy_no_clip(float(x1), float(y1), float(x2), float(y2), w, h),
            "confidence": float(score),
            "attributes": {}
        })

    if preview:
        vis = bgr.copy()
        for d in dets:
            x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], w, h)
            cv2.rectangle(vis, (x1, y1), (x2, y2), (0, 255, 0), 2)
            label = f"{d['confidence']:.2f}"
            cv2.putText(vis, label, (x1, max(0, y1 - 6)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 0), 2)
        os.makedirs(os.path.dirname(preview) or ".", exist_ok=True)
        cv2.imwrite(preview, vis)

    return dets



if __name__ == "__main__":
    import argparse, json
    from pathlib import Path

    parser = argparse.ArgumentParser(description="Run detectors on an image (CPU).")
    parser.add_argument("-i", "--input", required=True, help="Path to input image.")
    parser.add_argument(
        "-t", "--type",
        required=True,
        choices=["face", "plate"],
        help="Which detector to run."
    )
    parser.add_argument("-o", "--output", default="", help="Path to save json.")
    args = parser.parse_args()

    img_path = Path(args.input)
    if not img_path.exists():
        raise FileNotFoundError(f"Input image not found: {img_path}")

    if args.type == "face":
        dets = detect_faces(str(img_path))
    else:  # "plate"
        dets = detect_plates(str(img_path))

    # Save JSON to same stem name, with .json extension
    out_path = img_path.with_suffix(".json")
    if args.output:
        out_path = Path(args.output)
    os.makedirs(out_path.parent, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(dets, f, ensure_ascii=False, indent=2)

    print(f"Detections saved to {out_path}")


    # # --- simple test: read saved JSON and draw boxes back onto the original image ---

    # with open(out_path, "r", encoding="utf-8") as f:
    #     dets_loaded = json.load(f)

    # bgr = cv2.imread(str(img_path))
    # h, w = bgr.shape[:2]
    # vis = bgr.copy()

    # for d in dets_loaded:
    #     x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], w, h)
    #     # Color: blue if face (gender key present), else green (plate/other)
    #     is_face = bool(d.get("attributes", {}).get("gender"))
    #     color = (255, 0, 0) if is_face else (0, 255, 0)
    #     cv2.rectangle(vis, (x1, y1), (x2, y2), color, 2)
    #     label_parts = []
    #     if is_face:
    #         label_parts.append(d["attributes"]["gender"])
    #     conf = d.get("confidence", None)
    #     if conf is not None:
    #         label_parts.append(f"{conf:.2f}")
    #     if label_parts:
    #         cv2.putText(
    #             vis,
    #             " ".join(label_parts),
    #             (x1, max(0, y1 - 6)),
    #             cv2.FONT_HERSHEY_SIMPLEX,
    #             0.6,
    #             color,
    #             2,
    #         )

    # out_img = img_path.with_name(img_path.stem + "_vis.jpg")
    # cv2.imwrite(str(out_img), vis)
    # print(f"Preview saved to {out_img}")