#ifndef LICORNEA_KINECT_LIVE_CHECKERBOARD_H_
#define LICORNEA_KINECT_LIVE_CHECKERBOARD_H_

#include "../../lib/opencv.h"
#include "../../lib/obj_img_correspondence.h"
#include "../../lib/intrinsics.h"
#include <vector>

namespace tlz {

struct checkerboard {
	int cols = 0;
	int rows = 0;
	real square_width = 0.0;
	std::vector<vec2> corners;
	std::vector<vec2> outer_corners;
	cv::Rect bounding_rect;
	
	checkerboard() = default;
	checkerboard(int cols_, int rows_, real square_width_, const std::vector<vec2>& corners_);
	
	vec2 corner(int col, int row) const { return corners[cols*row + col]; }
	
	bool is_null() const { return corners.empty(); }
	explicit operator bool () const { return ! is_null(); }
	
	std::size_t corners_count() const { return (rows * cols); }
	
	std::vector<cv::Point> outer_corners_i() const;
};

struct checkerboard_extrinsics {
	vec3 translation;
	mat33 rotation;
	vec3 rotation_vec;
};

checkerboard detect_color_checkerboard(cv::Mat_<cv::Vec3b>&, int cols, int rows, real square_width = NAN);
checkerboard detect_ir_checkerboard(cv::Mat_<uchar>&, int cols, int rows, real square_width = NAN);

std::vector<vec3> checkerboard_world_corners(int cols, int rows, real square_width);
std::vector<vec2> checkerboard_image_corners(const checkerboard&);

checkerboard_extrinsics estimate_checkerboard_extrinsics(const checkerboard&, const intrinsics&);

struct checkerboard_pixel_depth_sample {
	vec2 coordinates;
	real measured_depth = NAN;
	real calculated_depth = NAN;
};
std::vector<checkerboard_pixel_depth_sample> checkerboard_pixel_depth_samples(const checkerboard&, const cv::Mat_<float>& depth_image, int granularity = 1);

void calculate_checkerboard_pixel_depths(const intrinsics&, const checkerboard_extrinsics&, std::vector<checkerboard_pixel_depth_sample>& inout_samples);

vec2 checkerboard_parallel_measures(const checkerboard&);
real calculate_parallel_checkerboard_depth(const checkerboard&, const intrinsics& intr);

real checkerboard_reprojection_error(const checkerboard& chk, const intrinsics&, const checkerboard_extrinsics&);

struct checkerboard_visualization_parameters {
	bool line = true;
	cv::Vec3b lines_color = cv::Vec3b(100, 100, 255);
	int line_thickness = 3;
};
cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<cv::Vec3b>&, const checkerboard&, const checkerboard_visualization_parameters& = checkerboard_visualization_parameters());
cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<uchar>&, const checkerboard&, const checkerboard_visualization_parameters& = checkerboard_visualization_parameters());

cv::Mat_<cv::Vec3b> visualize_checkerboard_pixel_samples(const cv::Mat_<cv::Vec3b>&, const std::vector<checkerboard_pixel_depth_sample>&, int rad = 1);
cv::Mat_<cv::Vec3b> visualize_checkerboard_pixel_samples(const cv::Mat_<uchar>&, const std::vector<checkerboard_pixel_depth_sample>&, int rad = 1);

obj_img_correspondences<1, 1> checkerboard_obj_img_correspondences(const checkerboard&);
obj_img_correspondences<1, 2> checkerboard_obj_2img_correspondences(const checkerboard& a, const checkerboard& b);

}

#endif
