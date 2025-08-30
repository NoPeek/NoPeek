// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import argparse, os, cv2, json
import numpy as np
import torch, random
from typing import List, Dict, Tuple, Sequence
from PIL import Image

print(torch.cuda.is_available(), torch.cuda.device_count(), torch.cuda.get_device_name(0) if torch.cuda.is_available() else "N/A")

def _set_seed(s: int):
    random.seed(s)
    np.random.seed(s)
    torch.manual_seed(s)
    torch.cuda.manual_seed_all(s)

def _round_hw(h: int, w: int, mult: int = 16, max_side: int | None = None) -> Tuple[int, int]:
    if max_side:
        scale = min(1.0, max_side / max(h, w))
        if scale < 1.0:
            h = int(round(h * scale)); w = int(round(w * scale))
    h = (h // mult) * mult
    w = (w // mult) * mult
    return max(h, mult), max(w, mult)

def _denorm_xyxy(b: Sequence[float], W: int, H: int) -> Tuple[int, int, int, int]:
    x1 = int(round(float(b[0]) * W)); y1 = int(round(float(b[1]) * H))
    x2 = int(round(float(b[2]) * W)); y2 = int(round(float(b[3]) * H))
    return x1, y1, x2, y2

def _assert_in_bounds(x1: int, y1: int, x2: int, y2: int, W: int, H: int, what: str):
    if x1 < 0 or y1 < 0 or x2 > W or y2 > H:
        raise ValueError(f"{what} exceeds image bounds {W}x{H}: {(x1, y1, x2, y2)}")
    if x2 <= x1 or y2 <= y1:
        raise ValueError(f"Invalid {what} geometry: {(x1, y1, x2, y2)}")

def _expand_bbox(x1: int, y1: int, x2: int, y2: int,
                 W: int, H: int,
                 pct: float | None = 0.08,
                 px: int | None = None,
                 what: str = "expanded bbox") -> Tuple[int, int, int, int]:
    bw, bh = x2 - x1, y2 - y1
    if px is not None:
        dx = dy = int(px)
    else:
        dx = int(round(bw * float(pct)))
        dy = int(round(bh * float(pct)))
    ex1, ey1, ex2, ey2 = x1 - dx, y1 - dy, x2 + dx, y2 + dy
    _assert_in_bounds(ex1, ey1, ex2, ey2, W, H, what)
    return ex1, ey1, ex2, ey2

def _ellipse_mask_from_bbox(h: int, w: int, box: Tuple[int, int, int, int],
                            grow: float, feather: int) -> np.ndarray:
    x1, y1, x2, y2 = box
    bw, bh = x2 - x1, y2 - y1
    cx, cy = x1 + bw // 2, y1 + bh // 2
    rx, ry = int(bw * (0.5 + grow)), int(bh * (0.55 + grow))
    _assert_in_bounds(cx - rx, cy - ry, cx + rx, cy + ry, w, h, "ellipse")
    m = np.zeros((h, w), np.uint8)
    cv2.ellipse(m, (cx, cy), (rx, ry), 0, 0, 360, 255, -1)
    if feather > 0:
        k = feather | 1
        m = cv2.GaussianBlur(m, (k, k), 0)
    return m

def _rect_mask_for_plate(H: int, W: int, box: Tuple[int, int, int, int], pad: int, feather: int) -> np.ndarray:
    x1, y1, x2, y2 = box
    _assert_in_bounds(x1 - pad, y1 - pad, x2 + pad, y2 + pad, W, H, "plate+pad")
    m = np.zeros((H, W), np.uint8)
    cv2.rectangle(m, (x1 - pad, y1 - pad), (x2 + pad, y2 + pad), 255, -1)
    if feather > 0:
        k = feather | 1
        m = cv2.GaussianBlur(m, (k, k), 0)
    return m

def _load_detections(json_path: str) -> List[Dict]:
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("Detections JSON must be a list of objects.")
    out: List[Dict] = []
    for d in data:
        if not isinstance(d, dict):
            continue
        bb = d.get("bbox_xyxy", None)
        if isinstance(bb, (list, tuple)) and len(bb) == 4:
            attrs = d.get("attributes", {})
            if not isinstance(attrs, dict):
                attrs = {}
            out.append({"bbox_xyxy": bb, "attributes": attrs})
    return out


# ---- lightweight size-based downsampler (JPEG-estimate) ----
def _encoded_size_bytes(bgr: np.ndarray, quality: int = 92) -> int:
    ok, buf = cv2.imencode(".jpg", bgr, [int(cv2.IMWRITE_JPEG_QUALITY), int(quality)])
    if not ok:
        raise RuntimeError("cv2.imencode failed while estimating size")
    return len(buf)

def _downsample_to_approx_bytes(
    bgr: np.ndarray,
    target_bytes: int = 1_000_000,   # ~1 MB
    min_side: int = 640,             # don’t shrink below this
    quality: int = 92                # encoding quality used only for size estimation
) -> np.ndarray:
    H, W = bgr.shape[:2]
    cur = _encoded_size_bytes(bgr, quality=quality)
    if cur <= target_bytes:
        return bgr

    # one-shot scale based on sqrt ratio of sizes
    scale = max((target_bytes / float(cur)) ** 0.5, 0.1)
    new_w = max(min_side, int(W * scale))
    new_h = max(min_side, int(H * scale))
    if new_w < W or new_h < H:
        bgr = cv2.resize(bgr, (new_w, new_h), interpolation=cv2.INTER_AREA)

    # refine with a short multiplicative backoff if still too big
    backoff = 0.88
    tries = 0
    while _encoded_size_bytes(bgr, quality=quality) > target_bytes and tries < 6:
        h, w = bgr.shape[:2]
        if h <= min_side or w <= min_side:
            break
        bgr = cv2.resize(
            bgr,
            (max(min_side, int(w * backoff)), max(min_side, int(h * backoff))),
            interpolation=cv2.INTER_AREA,
        )
        tries += 1
    return bgr


def inpaint_faces(bgr: np.ndarray,
                  dets: List[Dict],
                  engine: str = "flux_depth") -> np.ndarray:
    """
    Stronger anime stylization for *all* faces.
    - Heavier LoRA
    - Stronger overwrite (depth strength higher)
    - Larger mask & bbox expansion
    """
    if not dets:
        return bgr
    if not torch.cuda.is_available():
        raise RuntimeError("CUDA not available: the face pipeline requires a CUDA device for speed.")

    bgr = _downsample_to_approx_bytes(bgr, target_bytes=1_000_000, min_side=640, quality=92)

    from diffusers import FluxFillPipeline, FluxControlInpaintPipeline

    # ---- style block tuned for 'fake/anime' look ----
    base_prompt = (
        "anime avatar, 2D illustration, cel shading, flat colors, "
        "clean bold lineart, big glossy eyes, tiny nose and mouth, "
        "smooth plastic-like skin, soft specular highlights, kawaii aesthetic, "
        "consistent head pose, studio portrait"
    )
    # avoid photorealism by *describing* what we want, rather than using negative_prompt (not supported by some Flux calls)
    style_tags = [
        "uniform tones, minimal skin texture, no pores, no blemishes",
        "sharp silhouette, crisp contours, simplified hair clumps",
        "soft rim light, gentle bloom, shallow DOF",
        "high coherence, symmetric facial features"
    ]
    gender_map = {"male": "masculine anime facial features", "female": "feminine anime facial features", "neutral": ""}

    # More aggressive coverage & blending for a full replacement
    grow = 0.20     # was 0.12
    feather = 35    # slightly softer edges than 33
    max_side = 1280

    # LoRA (kept from your setup but increased influence)
    lora_repo   = "XLabs-AI/flux-lora-collection"
    lora_weight = "anime_lora.safetensors"
    lora_scale  = 1.15   # was 0.80

    seed = 1234

    # Sampling tuned for stronger stylization
    # Flux-Fill benefits from a few more steps & modest guidance
    fill_steps, fill_guidance = 60, 8.0          # was 44, 30.0 (30 is too high for stylization here)
    # Depth: increase overwrite strength to replace original features more decisively
    depth_steps, depth_guidance, depth_strength = 36, 7.0, 0.985  # was 30, 10.0, 0.90

    common = f"{base_prompt}, {', '.join(style_tags)}, ultra clean, high quality"

    H, W = bgr.shape[:2]
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
    h, w = _round_hw(H, W, mult=16, max_side=max_side)

    device = torch.device("cuda")
    major, _ = torch.cuda.get_device_capability(0)
    dtype = torch.float16 if major >= 7 else torch.float32

    _set_seed(seed)

    if engine == "flux_fill":
        repo = "black-forest-labs/FLUX.1-Fill-dev"
        pipe = FluxFillPipeline.from_pretrained(repo, torch_dtype=dtype)
        control_img = None
    elif engine == "flux_depth":
        # Depth preprocessor on CUDA
        from image_gen_aux import DepthPreprocessor
        depth_proc = DepthPreprocessor.from_pretrained("LiheYoung/depth-anything-large-hf")
        depth_proc = depth_proc.to(device)
        control_img = depth_proc(Image.fromarray(rgb))[0].convert("RGB")
        del depth_proc
        torch.cuda.empty_cache()

        repo = "black-forest-labs/FLUX.1-Depth-dev"
        pipe = FluxControlInpaintPipeline.from_pretrained(repo, torch_dtype=dtype)
    else:
        raise ValueError("engine must be 'flux_fill' or 'flux_depth'")

    # Heavier LoRA for anime look
    pipe.load_lora_weights(lora_repo, adapter_name="flux_lora", weight_name=lora_weight)
    if hasattr(pipe, "set_adapters"):
        pipe.set_adapters(["flux_lora"], adapter_weights=[float(lora_scale)])
    elif hasattr(pipe, "set_lora_scale"):
        pipe.set_lora_scale(float(lora_scale))
    pipe.to(device)  # ensure CUDA after LoRA load

    work = rgb.copy()
    for k, d in enumerate(dets):
        gtok = d.get("attributes", {}).get("gender", "neutral")
        gtxt = gender_map.get(gtok, "")
        prompt_i = f"{common}, {gtxt}" if gtxt else common

        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        # Expand more to swallow hairline/cheeks for a full anime replacement
        x1, y1, x2, y2 = _expand_bbox(x1, y1, x2, y2, W, H, pct=0.16, what=f"face {k} bbox (expanded)")
        mask_i = _ellipse_mask_from_bbox(H, W, (x1, y1, x2, y2), grow=grow, feather=feather)

        _set_seed((seed + 101 * k) & 0x7FFFFFFF)
        gen = torch.Generator(device=device).manual_seed(seed + 101 * k)

        if engine == "flux_fill":
            out = pipe(
                prompt=prompt_i,
                image=Image.fromarray(work),
                mask_image=Image.fromarray(mask_i).convert("L"),
                height=h, width=w,
                num_inference_steps=fill_steps,
                guidance_scale=fill_guidance,
                max_sequence_length=512,
                generator=gen,
            ).images[0]
        else:  # flux_depth
            out = pipe(
                prompt=prompt_i,
                image=Image.fromarray(work),
                mask_image=Image.fromarray(mask_i).convert("L"),
                control_image=control_img,
                height=h, width=w,
                num_inference_steps=depth_steps,
                guidance_scale=depth_guidance,
                strength=depth_strength,  # higher -> more “fake/anime” replacement
                generator=gen,
            ).images[0]

        out_np = np.array(out)
        if (h, w) != (H, W):
            out_np = cv2.resize(out_np, (W, H), interpolation=cv2.INTER_LANCZOS4)
        work = out_np

    return cv2.cvtColor(work, cv2.COLOR_RGB2BGR)


def inpaint_plates(bgr: np.ndarray, dets: List[Dict]) -> np.ndarray:
    if not dets:
        return bgr

    from diffusers import FluxFillPipeline

    prompt = ("Fake license plate with plausible but random numbers and letters, "
              "not corresponding to any real country, realistic style, seamlessly blending with the car.")
    steps = 44
    guidance = 28.0
    max_side = 1280
    seed = 1234
    pad = 6
    feather = 21

    lora_repo = None
    lora_weight = None
    lora_scale = 1.0

    H, W = bgr.shape[:2]
    work = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
    dtype, device = torch.float16, torch.device("cuda")
    h, w = _round_hw(H, W, mult=16, max_side=max_side)

    pipe = FluxFillPipeline.from_pretrained("black-forest-labs/FLUX.1-Fill-dev",
                                            torch_dtype=dtype).to(device)
    if lora_repo:
        load_kwargs = {}
        if lora_weight:
            load_kwargs["weight_name"] = lora_weight
        pipe.load_lora_weights(lora_repo, adapter_name="flux_lora", **load_kwargs)
        if hasattr(pipe, "set_adapters"):
            pipe.set_adapters(["flux_lora"], adapter_weights=[float(lora_scale)])
        elif hasattr(pipe, "set_lora_scale"):
            pipe.set_lora_scale(float(lora_scale))

    for idx, d in enumerate(dets):
        x1, y1, x2, y2 = _denorm_xyxy(d["bbox_xyxy"], W, H)
        mask = _rect_mask_for_plate(H, W, (x1, y1, x2, y2), pad=pad, feather=feather)

        _set_seed((seed + idx * 101) & 0x7FFFFFFF)
        gen = torch.Generator(device=device).manual_seed(seed + idx * 101)

        out = pipe(
            prompt=prompt,
            image=Image.fromarray(work),
            mask_image=Image.fromarray(mask).convert("L"),
            height=h, width=w,
            num_inference_steps=steps,
            guidance_scale=guidance,
            max_sequence_length=512,
            generator=gen,
        ).images[0]

        out_np = np.array(out)
        if (h, w) != (H, W):
            out_np = cv2.resize(out_np, (W, H), interpolation=cv2.INTER_LANCZOS4)
        work = out_np

    return cv2.cvtColor(work, cv2.COLOR_RGB2BGR)

def main():
    import time

    s_ = time.time()
    ap = argparse.ArgumentParser(description="Inpaint faces OR plates (one kind per run) using precomputed detections.")
    ap.add_argument("-i", "--input", required=True, help="Input image path")
    ap.add_argument("-j", "--json", required=True, help="Detections JSON (list of {bbox_xyxy:[x1,y1,x2,y2], ...})")
    ap.add_argument("-t", "--target", choices=["face", "plate"], required=True, help="What to inpaint")
    ap.add_argument("-o", "--output", default=None, help="Output image path")
    ap.add_argument("--engine", choices=["flux_fill", "flux_depth"], default="flux_depth",
                    help="Face inpaint engine (used only when -t face)")
    args = ap.parse_args()

    bgr = cv2.imread(args.input)
    if bgr is None:
        raise FileNotFoundError(args.input)

    dets: List[Dict] = _load_detections(args.json)

    if args.target == "face":
        if not dets:
            print("No face boxes in JSON — output will equal input.")
            out = bgr
        else:
            out = inpaint_faces(bgr, dets, engine=args.engine)
        suffix = "_face_inpaint.jpg"
    else:
        if not dets:
            print("No plate boxes in JSON — output will equal input.")
            out = bgr
        else:
            out = inpaint_plates(bgr, dets)
        suffix = "_plate_inpaint.jpg"

    out_path = args.output or os.path.splitext(args.input)[0] + suffix
    cv2.imwrite(out_path, out)
    print(f"Done. Wrote: {out_path}")
    print(f"Elapsed: {time.time() - s_:.1f} sec.")

if __name__ == "__main__":
    main()