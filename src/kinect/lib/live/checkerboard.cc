#include "checkerboard.h"
#include <algorithm>
#include <iostream>
#include <climits>
#include <cassert>

namespace tlz {
	
	
std::vector<cv::Point> checkerboard::outer_corners_i() const {
	std::vector<cv::Point> out_outer_corners_i(outer_corners.size());
	std::transform(outer_corners.begin(), outer_corners.end(), out_outer_corners_i.begin(), [](const cv::Point2f& pt) {
		return cv::Point(pt.x, pt.y);
	});
	return out_outer_corners_i;
}	
	
	
checkerboard detect_checkerboard(cv::Mat& img, int cols, int rows, real square_width) {
	std::vector<cv::Point2f> corners;

	int flags = CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK;
	bool found = cv::findChessboardCorners(img, cv::Size(cols, rows), corners, flags);
	if(!found || corners.size() != cols*rows) return checkerboard();
	
	cv::TermCriteria term(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1);
	if(img.channels() > 1) {
		cv::Mat img_mono;
		cv::cvtColor(img, img_mono, CV_BGR2GRAY);
		cv::cornerSubPix(img_mono, corners, cv::Size(11, 11), cv::Size(-1, -1), term);	
	} else {
		cv::cornerSubPix(img, corners, cv::Size(3, 3), cv::Size(-1, -1), term);	
	}
		
	checkerboard chk;
	chk.cols = cols;
	chk.rows = rows;
	chk.corners = corners;
	chk.square_width = square_width;
	chk.outer_corners = {
		corners.at(0),
		corners.at(cols-1),
		corners.at(cols*(rows-1) + (cols-1)),
		corners.at(cols*(rows-1))
	};
	
	int min_x = INT_MAX, min_y = INT_MAX, max_x = 0, max_y = 0;
	for(const cv::Point2f& pt : chk.outer_corners) {
		int x = pt.x, y = pt.y;
		if(x > max_x) max_x = x;
		if(x < min_x) min_x = x;
		if(y > max_y) max_y = y;
		if(y < min_y) min_y = y;
	}
	chk.bounding_rect = cv::Rect(min_x, min_y, max_x-min_x, max_y-min_y);
	
	return chk;
}


std::vector<vec3> checkerboard_world_corners(int cols, int rows, real square_width) {
	std::vector<vec3> world_corners(cols * rows);
	for(int row = 0, i = 0; row < rows; ++row) for(int col = 0; col < cols; ++col, ++i)
		world_corners[i] = vec3(col*square_width, row*square_width, 0.0);
	return world_corners;
}


std::vector<vec2> checkerboard_image_corners(const checkerboard& chk) {
	std::vector<vec2> image_corners;
	for(const cv::Point2f& pt : chk.corners) image_corners.emplace_back(pt.x, pt.y);
	return image_corners;	
}


cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<cv::Vec3b>& img, const checkerboard& chk, const checkerboard_visualization_parameters& param) {
	cv::Mat_<cv::Vec3b> out_img;
	img.copyTo(out_img);
	if(! chk) return out_img;

	if(param.line) {		
		std::vector<std::vector<cv::Point>> polylines { chk.outer_corners_i() };
		cv::polylines(out_img, polylines, true, cv::Scalar(param.lines_color), param.line_thickness);
	}
	
	return out_img;
}


cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<uchar>& img, const checkerboard& chk, const checkerboard_visualization_parameters& param) {
	cv::Mat_<cv::Vec3b> conv_img;
	cv::cvtColor(img, conv_img, CV_GRAY2BGR);
	return visualize_checkerboard(conv_img, chk, param);
}


obj_img_correspondences<1, 1> checkerboard_obj_img_correspondences(const checkerboard& chk) {
	obj_img_correspondences<1, 1> cors;
	std::vector<vec3> world_corners = checkerboard_world_corners(chk.cols, chk.rows, chk.square_width);
	std::vector<vec2> image_corners = checkerboard_image_corners(chk);
	for(int i = 0; i < chk.rows*chk.cols; ++i) {
		obj_img_correspondence<1, 1> cor;
		cor.object_coordinates[0] = world_corners[i];
		cor.image_coordinates[0] = image_corners[i];
		cors.push_back(cor);
	}
	return cors;
}


obj_img_correspondences<1, 2> checkerboard_obj_2img_correspondences(const checkerboard& chk1, const checkerboard& chk2) {
	assert(chk1.rows == chk2.rows && chk1.cols == chk2.cols && chk1.square_width == chk2.square_width);
	obj_img_correspondences<1, 2> cors;
	std::vector<vec3> world_corners = checkerboard_world_corners(chk1.cols, chk1.rows, chk1.square_width);
	std::vector<vec2> image1_corners = checkerboard_image_corners(chk1);
	std::vector<vec2> image2_corners = checkerboard_image_corners(chk2);
	for(int i = 0; i < chk1.rows*chk1.cols; ++i) {
		obj_img_correspondence<1, 2> cor;
		cor.object_coordinates[0] = world_corners[i];
		cor.image_coordinates[0] = image1_corners[i];
		cor.image_coordinates[1] = image2_corners[i];
		cors.push_back(cor);
	}
	return cors;
	
}

}
