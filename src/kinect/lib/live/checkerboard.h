#ifndef LICORNEA_KINECT_LIVE_CHECKERBOARD_H_
#define LICORNEA_KINECT_LIVE_CHECKERBOARD_H_

#include "../../lib/opencv.h"
#include "../../lib/obj_img_correspondence.h"
#include <vector>

namespace tlz {

struct checkerboard {
	int cols;
	int rows;
	real square_width;
	std::vector<cv::Point2f> corners;
	std::vector<cv::Point2f> outer_corners;
	cv::Rect bounding_rect;
	
	bool is_null() const { return corners.empty(); }
	explicit operator bool () const { return ! is_null(); }
	
	std::vector<cv::Point> outer_corners_i() const;
};
	
struct checkerboard_visualization_parameters {
	bool line = true;
	cv::Vec3b lines_color = cv::Vec3b(100, 100, 255);
	int line_thickness = 3;
};

checkerboard detect_checkerboard(cv::Mat&, int cols, int rows, real square_width = NAN);

std::vector<vec3> checkerboard_world_corners(int cols, int rows, real square_width);
std::vector<vec2> checkerboard_image_corners(const checkerboard&);

cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<cv::Vec3b>&, const checkerboard&, const checkerboard_visualization_parameters& = checkerboard_visualization_parameters());
cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<uchar>&, const checkerboard&, const checkerboard_visualization_parameters& = checkerboard_visualization_parameters());

obj_img_correspondences<1, 1> checkerboard_obj_img_correspondences(const checkerboard&);
obj_img_correspondences<1, 2> checkerboard_obj_2img_correspondences(const checkerboard& a, const checkerboard& b);

}

#endif
