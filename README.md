# lib-sfm — Customer Examples for libSFM

Sample integrations for consumers of the `libsfm` Debian package.
Each language binding lives in its own top-level directory so they can be
copied, built, and distributed independently.

```
lib-sfm/
├── README.md            # you are here
├── cpp/                 # C++ example (OpenCV)
│   ├── CMakeLists.txt
│   ├── README.md
│   └── src/stereo_example.cpp
├── python/              # (planned) Python example
├── input/               # supply your own frames here
│   ├── left.png         #   left  IR  (required)
│   ├── right.png        #   right IR  (required)
│   └── rgb.png          #   color frame (optional — enables colored PLY)
└── output/              # example outputs land here (gitignored)
```

The C++ example under [`cpp/`](cpp/README.md) reads the IR pair (and optional
color frame) from `input/` and writes a depth map plus a colored PLY file to
`output/`. A Python counterpart will be added under `python/` using the same
input/output layout.

## Prerequisites

libSFM ships as a Debian package on Ubuntu 22.04. Install the package
yourself, then make sure the supporting libraries below are present before
building any example.

| Category              | Packages                                                    |
|-----------------------|-------------------------------------------------------------|
| libSFM                | `libsfm`, `libsfm-dev`                                      |
| NVIDIA runtime        | CUDA Toolkit 12.9+, cuDNN 9.10.x, TensorRT 10.12.x          |
| C++ build tooling     | `build-essential`, `cmake ≥ 3.20`                           |
| C++ example deps      | `libopencv-dev`                                             |

## Supported GPUs

The distributed `libsfm.so` is compiled for compute capability **7.5 / 8.6 / 8.9**
(Turing, Ampere, Ada Lovelace). Other architectures require a rebuild — contact
PLAIF if needed.

## Supported resolutions

The following input resolutions are supported:

| Resolution  | Aspect ratio | Pixels   |
|-------------|--------------|----------|
| 640 × 360   | 16:9         | 0.23 MP  |
| 640 × 480   | 4:3          | 0.31 MP  |
| 1280 × 720  | 16:9         | 0.92 MP  |
| 1920 × 1080 | 16:9         | 2.07 MP  |

## Verified environments

Stable operation has been confirmed on the OS / kernel / driver combinations
below. Other combinations may also work but are not officially verified.

| OS              | Kernel              | NVIDIA driver | GPU             |
|-----------------|---------------------|---------------|-----------------|
| Ubuntu 22.04.5  | 6.8.0-107-generic   | 575.64.03     | RTX 5060        |
| Ubuntu 22.04.5  | 5.15.0-174-generic  | 580.95.05     | RTX 4070 Super  |

## First-run TensorRT engine build

On the first run (and whenever the GPU, driver, or TensorRT version changes),
libSFM compiles and serializes a TensorRT engine for your hardware. **Expect
at least ~20 minutes** of startup time for this step; exact duration depends on
your CPU, GPU, driver, and TensorRT version, and can be noticeably longer on
slower machines. Subsequent runs reuse the cached engine and start in seconds.

## Unit convention

The libSFM public API is millimeter-uniform. Baselines, extrinsic translations,
depth values, and point-cloud xyz are all in **mm**. When bridging to
RealSense or other meter-based sources, multiply translations by 1000 at the
boundary. See the `sfm` README for details.

## Support

Technical contact: `team_manipulation@plaif.com` (PLAIF Manipulation Team).
