/*
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
	float virtual_flat_depth_ = 1600.0;

	cv::Vec2f distort_depth_(cv::Vec2f undistorted) const;
	cv::Vec2f undistort_depth_(cv::Vec2f distorted) const;
	cv::Vec2f reproject_depth_to_color_(cv::Vec2f undistorted, float z) const;
	
public:
	explicit kinect_reprojection(const kinect_intrinsic_parameters&);
	/*
	cv::Mat_<cv::Vec3b> reproject_color(
		const cv::Mat_<cv::Vec3b>& color,
		cv::Size reprojected_size
	) const;

	std::vector<Eigen_vec3> reproject_depth(
		const cv::Mat_<float>& distorted_depth,
		cv::Size reprojected_size,
		float p
	) const;
	* /
	std::tuple<cv::Mat_<cv::Vec3b>, std::vector<Eigen_vec3>> reproject_dual(
		const cv::Mat_<cv::Vec3b>& color,
		const cv::Mat_<float>& distorted_depth,
		cv::Size reprojected_size
	) const;

};

}

#endif
*/
