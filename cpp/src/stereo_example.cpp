// Copyright (c) 2026 PLAIF. BSD-3-Clause.
// ============================================================================
// stereo_example.cpp
//
// libSFM customer-facing C++ example.
//
// Reads images from <project-root>/input/
//   left.png   : left  IR (BGR or grayscale; auto-loaded as BGR)   [required]
//   right.png  : right IR (same size as left.png)                  [required]
//   rgb.png    : color frame for point-cloud colorization          [optional]
//
// Writes artifacts to <project-root>/output/
//   depth_mm.png    : 16-bit single-channel raw depth (millimeters)
//   depth_color.png : 8-bit colorized depth preview (COLORMAP_JET)
//   pointcloud.ply  : ASCII PLY point cloud with per-vertex RGB
//
// ----------------------------------------------------------------------------
// IMPORTANT: the camera parameters below are calibrated for a D415 capture at
// 1280x720. Replace them if you use a different device, resolution, or rig.
// ============================================================================

#include <libsfm/libsfm.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace {

struct D415Calibration {
  static constexpr int kWidth = 1280;
  static constexpr int kHeight = 720;
  static constexpr sfm::Intrinsic kDepthIntrinsic{
      896.095458984375,
      896.095458984375,
      637.02587890625,
      363.90167236328125,
  };
  static constexpr double kBaselineMm = 55.079;

  static sfm::ColorCamera MakeColorCamera() {
    return {
        kWidth,
        kHeight,
        {
            912.5224609375,
            911.1925048828125,
            648.4265747070312,
            356.3631286621094,
        },
        {
            // API storage is column-major; the RealSense dump is usually shown
            // row-wise.
            {
                0.999969,
                0.001909,
                0.007650,
                -0.001945,
                0.999987,
                0.004676,
                -0.007641,
                -0.004691,
                0.999960,
            },
            {15.006, -0.125, -0.165},
        },
    };
  }
};

cv::Mat LoadBGR(const fs::path &path) {
  cv::Mat img = cv::imread(path.string(), cv::IMREAD_COLOR);
  if (img.empty())
    return img;
  if (!img.isContinuous())
    img = img.clone();
  return img;
}

void WarnIfDepthCalibrationMismatch(const cv::Size &size) {
  if (size.width == D415Calibration::kWidth &&
      size.height == D415Calibration::kHeight) {
    return;
  }

  std::cerr << "[warn] IR input is " << size.width << "x" << size.height
            << " but the baked-in D415 depth intrinsics are calibrated for "
            << D415Calibration::kWidth << "x" << D415Calibration::kHeight
            << "; recalibrate or update the depth intrinsics before trusting "
               "metric output\n";
}

void SaveDepth(const sfm::DepthMap &depth, const fs::path &out_dir) {
  // Zero-copy view into the library-owned buffer.
  const cv::Mat depth32(depth.height, depth.width, CV_32FC1,
                        const_cast<float *>(depth.data.data()));

  // 1) Raw 16-bit depth in millimeters — lossless, useful for downstream work.
  cv::Mat depth16;
  depth32.convertTo(depth16, CV_16UC1);
  cv::imwrite((out_dir / "depth_mm.png").string(), depth16);

  // 2) Colorized preview — invalid (<=0) pixels forced to black.
  const cv::Mat valid = depth32 > 0.0f;
  double vmin = 0.0, vmax = 0.0;
  cv::minMaxLoc(depth32, &vmin, &vmax, nullptr, nullptr, valid);
  if (vmax <= 0.0) {
    std::cerr << "[warn] depth map has no positive samples\n";
    return;
  }
  cv::Mat normalized;
  depth32.convertTo(normalized, CV_8UC1, -255.0 / vmax, 255.0);
  cv::Mat preview;
  cv::applyColorMap(normalized, preview, cv::COLORMAP_JET);
  preview.setTo(cv::Scalar(0, 0, 0), ~valid);
  cv::imwrite((out_dir / "depth_color.png").string(), preview);
}

bool SavePLY(const sfm::PointCloud &pc, const fs::path &path) {
  std::ofstream ofs(path);
  if (!ofs.is_open()) {
    std::cerr << "[error] cannot open " << path << " for write\n";
    return false;
  }

  const size_t n = pc.size();
  ofs << "ply\n"
      << "format ascii 1.0\n"
      << "element vertex " << n << "\n"
      << "property float x\n"
      << "property float y\n"
      << "property float z\n"
      << "property uchar red\n"
      << "property uchar green\n"
      << "property uchar blue\n"
      << "end_header\n";

  // PointCloud::points  : stride-3 xyz in mm
  // PointCloud::colors  : stride-3 rgb in [0, 1] (multiply by 255 for 8-bit)
  for (size_t i = 0; i < n; ++i) {
    const size_t k = i * 3;
    const int r =
        std::clamp(static_cast<int>(pc.colors[k + 0] * 255.0 + 0.5), 0, 255);
    const int g =
        std::clamp(static_cast<int>(pc.colors[k + 1] * 255.0 + 0.5), 0, 255);
    const int b =
        std::clamp(static_cast<int>(pc.colors[k + 2] * 255.0 + 0.5), 0, 255);
    ofs << pc.points[k + 0] << ' ' << pc.points[k + 1] << ' '
        << pc.points[k + 2] << ' ' << r << ' ' << g << ' ' << b << '\n';
  }
  return true;
}

void PrintSummary(const cv::Size &input_size, bool use_color,
                  const sfm::DepthMap &depth, const sfm::PointCloud &cloud,
                  const fs::path &output_dir) {
  std::cout << "input      = " << input_size.width << "x" << input_size.height
            << "  color=" << std::boolalpha << use_color << "\n"
            << "depth      = " << depth.width << "x" << depth.height << " ("
            << depth.size() << " samples, mm)\n"
            << "pointcloud = " << cloud.size() << " vertices\n"
            << "wrote: " << output_dir
            << "/{depth_mm.png, depth_color.png, pointcloud.ply}\n";
}

} // namespace

int main(int argc, char *argv[]) {
  int gpu_id = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      std::cout
          << "Usage: " << argv[0] << " [gpu-id]\n"
          << "Reads left.png/right.png (and optional rgb.png) from "
          << "<project>/input/ and writes results to <project>/output/.\n";
      return 0;
    }
    gpu_id = std::atoi(argv[i]);
  }

  // EXAMPLE_PROJECT_ROOT is injected by CMake so the binary can be launched
  // from any working directory.
  const fs::path project_root = fs::path(EXAMPLE_PROJECT_ROOT);
  const fs::path input_dir = project_root / "input";
  const fs::path output_dir = project_root / "output";
  fs::create_directories(output_dir);

  cv::Mat ir_left = LoadBGR(input_dir / "left.png");
  cv::Mat ir_right = LoadBGR(input_dir / "right.png");
  cv::Mat color = LoadBGR(input_dir / "rgb.png"); // optional

  if (ir_left.empty() || ir_right.empty()) {
    std::cerr << "[error] missing left.png or right.png in " << input_dir
              << "\n";
    return 1;
  }
  if (ir_left.size() != ir_right.size()) {
    std::cerr << "[error] left/right resolution mismatch: " << ir_left.cols
              << "x" << ir_left.rows << " vs " << ir_right.cols << "x"
              << ir_right.rows << "\n";
    return 1;
  }

  const cv::Size ir_size = ir_left.size();
  const bool use_color = !color.empty();

  WarnIfDepthCalibrationMismatch(ir_size);
  const sfm::ColorCamera color_cam = D415Calibration::MakeColorCamera();

  auto &proc = sfm::SFMProcess::Instance();
  if (!proc.Initialize(gpu_id)) {
    std::cerr << "[error] sfm::SFMProcess::Initialize failed (gpu=" << gpu_id
              << ")\n";
    return 1;
  }

  sfm::DepthMap depth;
  sfm::PointCloud cloud;

  const int status = proc.Infer(
    ir_left.data,
    ir_right.data,
    ir_size.width,
    ir_size.height,
    D415Calibration::kDepthIntrinsic,
    D415Calibration::kBaselineMm,
    &depth,
    &cloud,
    use_color ? color.data : nullptr,
    use_color ? &color_cam : nullptr,
    use_color ? color.cols : 0,
    use_color ? color.rows : 0);

  if (status != 0) {
    std::cerr << "[error] sfm::SFMProcess::Infer failed with status=" << status
              << "\n";
    proc.Finalize();
    return status;
  }

  SaveDepth(depth, output_dir);
  SavePLY(cloud, output_dir / "pointcloud.ply");
  PrintSummary(ir_size, use_color, depth, cloud, output_dir);

  proc.Finalize();
  return 0;
}
