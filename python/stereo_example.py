# Copyright (c) 2026 PLAIF. BSD-3-Clause.
# ============================================================================
# stereo_example.py
#
# libSFM customer-facing Python example. Parallel to cpp/src/stereo_example.cpp.
#
# Reads images from <repo-root>/input/
#   left.png   : left  IR (BGR or grayscale; auto-loaded as BGR)   [required]
#   right.png  : right IR (same size as left.png)                  [required]
#   rgb.png    : color frame for point-cloud colorization          [optional]
#
# Writes artifacts to <repo-root>/output/
#   depth_mm.png    : 16-bit single-channel raw depth (millimeters)
#   depth_color.png : 8-bit colorized depth preview (JET)
#   pointcloud.ply  : ASCII PLY point cloud with per-vertex RGB
#
# ----------------------------------------------------------------------------
# IMPORTANT: the camera parameters below are calibrated for a D415 capture at
# 1280x720. Replace them if you use a different device, resolution, or rig.
# ============================================================================
from __future__ import annotations

import sys
from pathlib import Path

import cv2
import numpy as np

import pysfm


# --- D415 @ 1280x720 calibration --------------------------------------------

IR_WIDTH = 1280
IR_HEIGHT = 720
DEPTH_INTRINSIC = pysfm.Intrinsic(
    fx=896.095458984375,
    fy=896.095458984375,
    cx=637.02587890625,
    cy=363.90167236328125,
)
BASELINE_MM = 55.079

COLOR_INTRINSIC = pysfm.Intrinsic(
    fx=912.5224609375,
    fy=911.1925048828125,
    cx=648.4265747070312,
    cy=356.3631286621094,
)
# libSFM stores the rotation column-major; the RealSense dump below is the
# usual row-major presentation. We transpose before writing so the flat
# 9-element buffer libSFM sees is in column-major order.
_COLOR_ROTATION_ROW_MAJOR = np.array(
    [
        [ 0.999969, -0.001945, -0.007641],
        [ 0.001909,  0.999987, -0.004691],
        [ 0.007650,  0.004676,  0.999960],
    ],
    dtype=np.float64,
)
COLOR_TRANSLATION_MM = np.array([15.006, -0.125, -0.165], dtype=np.float64)


def make_color_camera() -> pysfm.ColorCamera:
    extrinsic = pysfm.Extrinsic()
    # libSFM interprets rotation as column-major; flatten the transpose
    # so the 9-element buffer matches that convention.
    extrinsic.rotation = np.ascontiguousarray(
        _COLOR_ROTATION_ROW_MAJOR.T.reshape(-1)
    )
    extrinsic.translation = COLOR_TRANSLATION_MM

    cam = pysfm.ColorCamera()
    cam.width = IR_WIDTH
    cam.height = IR_HEIGHT
    cam.intrinsic = COLOR_INTRINSIC
    cam.depth_to_color = extrinsic
    return cam


# --- I/O helpers ------------------------------------------------------------

def load_bgr(path: Path) -> np.ndarray | None:
    if not path.is_file():
        return None
    img = cv2.imread(str(path), cv2.IMREAD_COLOR)
    if img is None:
        return None
    if not img.flags["C_CONTIGUOUS"]:
        img = np.ascontiguousarray(img)
    return img


def warn_calibration_mismatch(size: tuple[int, int]) -> None:
    h, w = size
    if w == IR_WIDTH and h == IR_HEIGHT:
        return
    print(
        f"[warn] IR input is {w}x{h} but the baked-in D415 depth intrinsics "
        f"are calibrated for {IR_WIDTH}x{IR_HEIGHT}; recalibrate or update the "
        f"depth intrinsics before trusting metric output",
        file=sys.stderr,
    )


def save_depth(depth_mm: np.ndarray, out_dir: Path) -> None:
    # 1) Raw 16-bit depth in millimeters — lossless, useful for downstream work.
    depth16 = np.clip(depth_mm, 0, np.iinfo(np.uint16).max).astype(np.uint16)
    cv2.imwrite(str(out_dir / "depth_mm.png"), depth16)

    # 2) Colorized preview — invalid (<=0) pixels forced to black.
    valid = depth_mm > 0.0
    if not valid.any():
        print("[warn] depth map has no positive samples", file=sys.stderr)
        return
    vmax = float(depth_mm[valid].max())
    if vmax <= 0.0:
        print("[warn] depth map has no positive samples", file=sys.stderr)
        return
    # Near=bright, far=dim (matches the cpp example's -255/vmax convention).
    scaled = np.clip(255.0 - depth_mm * (255.0 / vmax), 0.0, 255.0)
    scaled = scaled.astype(np.uint8)
    preview = cv2.applyColorMap(scaled, cv2.COLORMAP_JET)
    preview[~valid] = (0, 0, 0)
    cv2.imwrite(str(out_dir / "depth_color.png"), preview)


def save_ply(points_mm: np.ndarray, colors_01: np.ndarray, path: Path) -> None:
    # pysfm returns (N, 3) float64 arrays; colors are in [0, 1].
    assert points_mm.shape == colors_01.shape
    assert points_mm.ndim == 2 and points_mm.shape[1] == 3

    rgb_u8 = np.clip(colors_01 * 255.0 + 0.5, 0, 255).astype(np.uint8)
    n = points_mm.shape[0]

    with path.open("w") as f:
        f.write(
            "ply\n"
            "format ascii 1.0\n"
            f"element vertex {n}\n"
            "property float x\n"
            "property float y\n"
            "property float z\n"
            "property uchar red\n"
            "property uchar green\n"
            "property uchar blue\n"
            "end_header\n"
        )
        for i in range(n):
            x, y, z = points_mm[i]
            r, g, b = rgb_u8[i]
            f.write(f"{x} {y} {z} {int(r)} {int(g)} {int(b)}\n")


def print_summary(
    input_size: tuple[int, int],
    use_color: bool,
    depth_mm: np.ndarray,
    points: np.ndarray,
    output_dir: Path,
) -> None:
    h, w = input_size
    dh, dw = depth_mm.shape
    print(f"input      = {w}x{h}  color={use_color}")
    print(f"depth      = {dw}x{dh} ({depth_mm.size} samples, mm)")
    print(f"pointcloud = {points.shape[0]} vertices")
    print(f"wrote: {output_dir}/{{depth_mm.png, depth_color.png, pointcloud.ply}}")


# --- Entry point ------------------------------------------------------------

def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    input_dir = repo_root / "input"
    output_dir = repo_root / "output"
    output_dir.mkdir(parents=True, exist_ok=True)

    ir_left = load_bgr(input_dir / "left.png")
    ir_right = load_bgr(input_dir / "right.png")
    color = load_bgr(input_dir / "rgb.png")  # optional

    if ir_left is None or ir_right is None:
        print(
            f"[error] missing left.png or right.png in {input_dir}",
            file=sys.stderr,
        )
        return 1
    if ir_left.shape != ir_right.shape:
        print(
            f"[error] left/right resolution mismatch: "
            f"{ir_left.shape[1]}x{ir_left.shape[0]} vs "
            f"{ir_right.shape[1]}x{ir_right.shape[0]}",
            file=sys.stderr,
        )
        return 1

    ir_size = (ir_left.shape[0], ir_left.shape[1])
    use_color = color is not None
    warn_calibration_mismatch(ir_size)

    sfm = pysfm.SFMProcess.instance()
    if not sfm.initialize(gpu_id=0):
        print("[error] pysfm.SFMProcess.initialize failed", file=sys.stderr)
        return 1

    try:
        infer_kwargs: dict = dict(
            want_depth=True,
            want_pointcloud=True,
        )
        if use_color:
            infer_kwargs["bgr"] = color
            infer_kwargs["color"] = make_color_camera()

        out = sfm.infer(
            ir_left,
            ir_right,
            DEPTH_INTRINSIC,
            BASELINE_MM,
            **infer_kwargs,
        )
    finally:
        sfm.finalize()

    depth_mm = out["depth"]
    points = out["points"]
    colors = out["colors"]

    save_depth(depth_mm, output_dir)
    save_ply(points, colors, output_dir / "pointcloud.ply")
    print_summary(ir_size, use_color, depth_mm, points, output_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
