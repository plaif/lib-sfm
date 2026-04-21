# libSFM — C++ Example

Sample program that drives `sfm::SFMProcess` on the IR pair under `../input/`
and writes a depth map plus a colored point cloud to `../output/`.

## Prerequisites

Install the Debian packages listed in the repository-root [`README.md`](../README.md)
(`libsfm-dev`, `libopencv-dev`, CUDA / TensorRT / cuDNN runtimes). This example
assumes `find_package(sfm)` can locate `sfm-config.cmake`, which `libsfm-dev`
installs under `/usr/lib/x86_64-linux-gnu/cmake/sfm/`.

## Build

```bash
cd cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Inputs

Drop your captured frames into `../input/`:

| File        | Required | Notes                                                     |
|-------------|----------|-----------------------------------------------------------|
| `left.png`  | yes      | Left  IR frame (BGR or grayscale; loaded as BGR)          |
| `right.png` | yes      | Right IR frame, same resolution as `left.png`             |
| `rgb.png`   | no       | Color frame; when present, point cloud is colorized       |

The example is currently preloaded with a **1280×720 Intel RealSense D415**
calibration:

- Depth / IR intrinsics: `fx=896.095458984375`, `fy=896.095458984375`,
  `cx=637.02587890625`, `cy=363.90167236328125`
- Color intrinsics: `fx=912.5224609375`, `fy=911.1925048828125`,
  `cx=648.4265747070312`, `cy=356.3631286621094`
- Stereo baseline: `55.079 mm`
- Depth → color translation: `[15.006, -0.125, -0.165] mm`

For other resolutions or a different rig, edit the calibration constants in
[`src/stereo_example.cpp`](src/stereo_example.cpp) (search for
`Camera parameters`).

## Run

```bash
./build/stereo_example      # GPU 0
./build/stereo_example 1    # GPU 1
```

Outputs (overwritten each run) are written to `../output/`:

| File              | Description                                         |
|-------------------|-----------------------------------------------------|
| `depth_mm.png`    | 16-bit single-channel depth, raw millimeters        |
| `depth_color.png` | 8-bit JET colormap preview (invalid pixels black)   |
| `pointcloud.ply`  | ASCII PLY with per-vertex RGB                       |

## Source layout

```
cpp/
├── CMakeLists.txt              # find_package(sfm) + find_package(OpenCV)
├── README.md
└── src/
    └── stereo_example.cpp      # load images → Infer() → save depth + PLY
```

## Calibration warning

`stereo_example.cpp` now contains one concrete D415 calibration at `1280×720`.
Metric output is only valid when your input frames match that device/resolution
pair. Replace the values in `Camera parameters` for any other setup.

See the libSFM upstream README for the full API reference and unit conventions
(millimeters throughout).
