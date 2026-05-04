# libSFM — Python Example

Python counterpart of [`cpp/`](../cpp/README.md). Drives `pysfm.SFMProcess`
on the IR pair under `../input/` and writes a depth map plus a colored point
cloud to `../output/`.

## Prerequisites

- System packages and NVIDIA runtime listed in the
  [repository-root README](../README.md) (CUDA Toolkit 12.9, cuDNN 9.10.x, TensorRT 10.12.x, plus `libcurl4 / libssl3 / libfmt8`).
- Python 3.10 (the wheel is tagged `cp310`).
- `pysfm` wheel installed with the `opencv` extra:

  ```bash
  pip install numpy opencv-python
  # ... and pysfm-<version>-cp310-cp310-linux_x86_64.whl download from Release page,
  # install it as follows
  pip install pysfm-<version>-cp310-cp310-linux_x86_64.whl opencv-python
  ```

  The wheel version tracks libSFM's `version.txt`, so the filename bumps in
  lock-step with the C++ library — substitute the version printed by the
  shipped wheel for `<version>`.

  See the root-README section [Python package (pysfm)](../README.md#python-package-pysfm)
  for the full install matrix (apt / tar / pip-wheel NVIDIA stacks).

## Inputs

Drop your captured frames into `../input/`:

| File        | Required | Notes                                                     |
|-------------|----------|-----------------------------------------------------------|
| `left.png`  | yes      | Left  IR frame (BGR or grayscale; loaded as BGR)          |
| `right.png` | yes      | Right IR frame, same resolution as `left.png`             |
| `rgb.png`   | no       | Color frame; when present, point cloud is colorized       |

The script is currently preloaded with a **1280×720 Intel RealSense D415**
calibration:

- Depth / IR intrinsics: `fx=896.095458984375`, `fy=896.095458984375`,
  `cx=637.02587890625`, `cy=363.90167236328125`
- Color intrinsics: `fx=912.5224609375`, `fy=911.1925048828125`,
  `cx=648.4265747070312`, `cy=356.3631286621094`
- Stereo baseline: `55.079 mm`
- Depth → color translation: `[15.006, -0.125, -0.165] mm`

For other resolutions or a different rig, edit the constants at the top of
[`stereo_example.py`](stereo_example.py) (`DEPTH_INTRINSIC`, `COLOR_*`,
`BASELINE_MM`).

## Run

```bash
python stereo_example.py
```

Outputs (overwritten each run) are written to `../output/`:

| File              | Description                                         |
|-------------------|-----------------------------------------------------|
| `depth_mm.png`    | 16-bit single-channel depth, raw millimeters        |
| `depth_color.png` | 8-bit JET colormap preview (invalid pixels black)   |
| `pointcloud.ply`  | ASCII PLY with per-vertex RGB                       |

## Source layout

```
python/
├── README.md
└── stereo_example.py   # load images → SFMProcess.infer() → save depth + PLY
```

## Calibration warning

`stereo_example.py` contains one concrete D415 calibration at `1280×720`.
Metric output is only valid when your input frames match that device /
resolution pair. Replace the values in the calibration block at the top of
the file for any other setup.

See the libSFM upstream README for the full API reference and unit
conventions (millimeters throughout).
