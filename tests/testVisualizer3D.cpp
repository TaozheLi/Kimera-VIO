/* ----------------------------------------------------------------------------
 * Copyright 2017, Massachusetts Institute of Technology,
 * Cambridge, MA 02139
 * All Rights Reserved
 * Authors: Luca Carlone, et al. (see THANKS for the full author list)
 * See LICENSE for the license information
 * -------------------------------------------------------------------------- */

/**
 * @file   testVisualizer3D.cpp
 * @brief  test Visualizer3D
 * @author Antoni Rosinol
 */

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "kimera-vio/backend/VioBackEnd-definitions.h"
#include "kimera-vio/frontend/StereoVisionFrontEnd-definitions.h"
#include "kimera-vio/mesh/Mesher-definitions.h"
#include "kimera-vio/utils/ThreadsafeQueue.h"
#include "kimera-vio/visualizer/Visualizer3D.h"

DECLARE_string(test_data_path);

namespace VIO {

class VisualizerFixture : public ::testing::Test {
 public:
  VisualizerFixture()
      : img_(),
        img_name_(),
        frame_(nullptr),
        viz_type_(VisualizationType::kMesh2dTo3dSparse),
        backend_type_(BackendType::kStereoImu),
        visualizer_(nullptr) {
    img_name_ = std::string(FLAGS_test_data_path) + "/chessboard_small.png";
    img_ = UtilsOpenCV::ReadAndConvertToGrayScale(img_name_);
    // Construct a frame from image name, and extract keypoints/landmarks.
    frame_ = constructFrame(true);
    visualizer_ = VIO::make_unique<Visualizer3D>(viz_type_, backend_type_);
  }

 protected:
  virtual void SetUp() override {}
  virtual void TearDown() override {}

 private:
  std::unique_ptr<Frame> constructFrame(bool extract_corners) {
    // Construct a frame from image name.
    FrameId id = 0;
    Timestamp tmp = 123;

    std::unique_ptr<Frame> frame =
        VIO::make_unique<Frame>(id, tmp, CameraParams(), img_);

    if (extract_corners) {
      frame->extractCorners();
      // Populate landmark structure with fake data.
      for (int i = 0; i < frame->keypoints_.size(); i++) {
        frame->landmarks_.push_back(i);
      }
    }

    return frame;
  }

 protected:
  static constexpr double tol = 1e-8;

  cv::Mat img_;
  std::string img_name_;
  std::unique_ptr<Frame> frame_;
  VisualizationType viz_type_;
  BackendType backend_type_;
  Visualizer3D::UniquePtr visualizer_;
};

TEST_F(VisualizerFixture, spinOnce) {
  Timestamp timestamp = 0;
  MesherOutput::Ptr mesher_output = std::make_shared<MesherOutput>(timestamp);
  BackendOutput::Ptr backend_output =
      std::make_shared<BackendOutput>(timestamp,
                                      gtsam::Values(),
                                      gtsam::Pose3(),
                                      Vector3(),
                                      ImuBias(),
                                      gtsam::Matrix(),
                                      FrameId(),
                                      0,
                                      DebugVioInfo(),
                                      PointsWithIdMap(),
                                      LmkIdToLmkTypeMap());
  FrontendOutput::Ptr frontend_output = std::make_shared<FrontendOutput>(
      true,
      StatusStereoMeasurements(),
      TrackingStatus(),
      gtsam::Pose3(),
      StereoFrame(FrameId(),
                  timestamp,
                  cv::Mat(),
                  CameraParams(),
                  cv::Mat(),
                  CameraParams(),
                  StereoMatchingParams()),
      ImuFrontEnd::PreintegratedImuMeasurements(),
      DebugTrackerInfo());
  VisualizerInput visualizer_input(
      timestamp, mesher_output, backend_output, frontend_output);
  // Visualize mesh.
  EXPECT_NO_THROW(visualizer_->spinOnce(visualizer_input));
}

}  // namespace VIO
