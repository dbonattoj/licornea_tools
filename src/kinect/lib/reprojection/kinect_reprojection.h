#ifndef LICORNEA_KINECT_REPROJECTION_H_
#define LICORNEA_KINECT_REPROJECTION_H_

#include "../../../lib/opencv.h"
#include "../../../lib/color.h"
#include "../../../lib/eigen.h"
#include "../kinect_intrinsics.h"
#include <vector>
#include <tuple>

namespace tlz {

class kinect_reprojection {
private:
	kinect_intrinsic_parameters intrinsics_;
	
public:
	explicit kinect_reprojection(const kinect_intrinsic_parameters&);
	
	cv::Vec2f distort_depth(cv::Vec2f undistorted) const;
	cv::Vec2f undistort_depth(cv::Vec2f distorted) const;
	cv::Vec2f reproject_depth_to_color(cv::Vec2f undistorted, float z) const;
	cv::Vec3f backproject_depth(cv::Vec2f undistorted, float z) const;
};

}

#endif
