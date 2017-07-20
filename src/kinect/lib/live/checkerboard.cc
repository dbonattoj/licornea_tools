#include "checkerboard.h"
#include "../../lib/common.h"
#include "../../lib/misc.h"
#include <algorithm>
#include <iostream>
#include <climits>
#include <cassert>

#if 0

// based on
// https://github.com/code-iai/iai_kinect2/blob/master/kinect2_calibration/src/kinect2_calibration.cpp


namespace tlz {

checkerboard::checkerboard(int cols_, int rows_, real square_width_, const std::vector<vec2>& corners_) :
	cols(cols_),
	rows(rows_),
	square_width(square_width_),
	corners(corners_)
{
	assert(corners_.size() == cols*rows);
	
	outer_corners = {
		corner(0, 0),
		corner(cols-1, 0),
		corner(cols-1, rows-1),
		corner(0, rows-1)
	};
	
	int min_x = INT_MAX, min_y = INT_MAX, max_x = 0, max_y = 0;
	for(const vec2& pt : outer_corners) {
		int x = pt[0], y = pt[1];
		if(x > max_x) max_x = x;
		if(x < min_x) min_x = x;
		if(y > max_y) max_y = y;
		if(y < min_y) min_y = y;
	}
	bounding_rect = cv::Rect(min_x, min_y, max_x-min_x, max_y-min_y);
}
	
std::vector<cv::Point> checkerboard::outer_corners_i() const {
	return vec2_to_point(outer_corners);
}
	
	
checkerboard detect_color_checkerboard(cv::Mat_<cv::Vec3b>& img, int cols, int rows, real square_width) {
	std::vector<cv::Point2f> corners;

	int flags = cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH;
	bool found = cv::findChessboardCorners(img, cv::Size(cols, rows), corners, flags);
	if(!found || corners.size() != cols*rows) return checkerboard();
	
	cv::TermCriteria term(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 100, DBL_EPSILON);
	cv::Mat img_mono;
	cv::cvtColor(img, img_mono, CV_BGR2GRAY);
	cv::cornerSubPix(img_mono, corners, cv::Size(11, 11), cv::Size(-1, -1), term);	
	
	return checkerboard(cols, rows, square_width, point2f_to_vec2(corners));
}


checkerboard detect_ir_checkerboard(cv::Mat_<uchar>& img, int cols, int rows, real square_width) {	
	static cv::Ptr<cv::CLAHE> clahe = nullptr;
	if(! clahe) clahe = cv::createCLAHE(1.5, cv::Size(32, 32));
	
	cv::Mat_<uchar> larger_img;
	real scale = 2.0;
	cv::resize(img, larger_img, cv::Size(0,0), scale, scale, cv::INTER_CUBIC);
	
	clahe->apply(larger_img, larger_img);
	
	std::vector<cv::Point2f> corners;

	int flags = cv::CALIB_CB_ADAPTIVE_THRESH;
	bool found = cv::findChessboardCorners(larger_img, cv::Size(cols, rows), corners, flags);
	if(!found || corners.size() != cols*rows) return checkerboard();
		
	cv::TermCriteria term(cv::TermCriteria::EPS, 100, DBL_EPSILON);
	cv::cornerSubPix(larger_img, corners, cv::Size(11, 11), cv::Size(-1, -1), term);	

	for(cv::Point2f& corner : corners) corner = cv::Point2f(corner.x/scale, corner.y/scale);
		
	return checkerboard(cols, rows, square_width, point2f_to_vec2(corners));
}


checkerboard detect_ir_checkerboard(cv::Mat_<ushort>& ir_orig, int cols, int rows, real square_width) {
	ushort max_ir = 0xffff;
	ushort min_ir = 0;
	float alpha = 255.0f / (max_ir - min_ir);
	float beta = -alpha * min_ir;
	cv::Mat_<uchar> ir;
	cv::convertScaleAbs(ir_orig, ir, alpha, beta);
	//ir.setTo(0, (ir_orig < min_ir));
	//ir.setTo(255, (ir_orig > max_ir));
	ir.setTo(0, (ir_orig == 0));
	return detect_ir_checkerboard(ir, cols, rows, square_width);
}


std::vector<vec3> checkerboard_world_corners(int cols, int rows, real square_width) {
	std::vector<vec3> world_corners(cols * rows);
	for(int row = 0, idx = 0; row < rows; ++row) for(int col = 0; col < cols; ++col, ++idx)
		world_corners[idx] = vec3((col - cols/2)*square_width, (row - rows/2)*square_width, 0.0);
	return world_corners;
}


std::vector<vec2> checkerboard_image_corners(const checkerboard& chk) {
	return chk.corners;
}


cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<cv::Vec3b>& img, const checkerboard& chk, const checkerboard_visualization_parameters& param) {
	cv::Mat_<cv::Vec3b> out_img;
	img.copyTo(out_img);
	if(! chk) return out_img;

	if(param.line) {		
		std::vector<std::vector<cv::Point>> polylines { chk.outer_corners_i() };
		cv::polylines(out_img, polylines, true, cv::Scalar(param.lines_color), param.line_thickness);
	}
	
	if(param.cv_visualization) {
		cv::drawChessboardCorners(out_img, cv::Size(chk.cols, chk.rows), vec2_to_point2f(chk.corners), true);
	}
	
	return out_img;
}


cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<uchar>& img, const checkerboard& chk, const checkerboard_visualization_parameters& param) {
	cv::Mat_<cv::Vec3b> conv_img;
	cv::cvtColor(img, conv_img, CV_GRAY2BGR);
	return visualize_checkerboard(conv_img, chk, param);
}
cv::Mat_<cv::Vec3b> visualize_checkerboard(const cv::Mat_<ushort>& img, const checkerboard& chk, const checkerboard_visualization_parameters& param) {
	cv::Mat_<uchar> img_8bit(img);
	ushort max_ir = 0xffff;
	ushort min_ir = 0;
	float alpha = 255.0f / (max_ir - min_ir);
	float beta = -alpha * min_ir;
	cv::Mat_<uchar> ir;
	cv::convertScaleAbs(img, img_8bit, alpha, beta);
	return visualize_checkerboard(img_8bit, chk, param);
}


cv::Mat_<cv::Vec3b> visualize_checkerboard_pixel_samples(const cv::Mat_<cv::Vec3b>& img, const std::vector<checkerboard_pixel_depth_sample>& pixels, int rad) {
	cv::Mat_<cv::Vec3b> out_img;
	img.copyTo(out_img);
	for(const auto& pix : pixels) {
		int x = pix.coordinates[0], y = pix.coordinates[1];
		if(x < 0 || x >= img.cols || y < 0 || y >= img.rows) continue;
		
		cv::Vec3b col(100, 100, 255);
		if(rad == 1) out_img(y, x) = col;
		else cv::circle(out_img, cv::Point(x, y), rad, cv::Scalar(col), 2);
	}
	return out_img;
}


cv::Mat_<cv::Vec3b> visualize_checkerboard_pixel_samples(const cv::Mat_<uchar>& img, const std::vector<checkerboard_pixel_depth_sample>& pixels, int rad) {
	cv::Mat_<cv::Vec3b> conv_img;
	cv::cvtColor(img, conv_img, CV_GRAY2BGR);
	return visualize_checkerboard_pixel_samples(conv_img, pixels, rad);
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


checkerboard_extrinsics estimate_checkerboard_extrinsics(const checkerboard& chk, const intrinsics& intr) {
	std::vector<vec3> object_points = checkerboard_world_corners(chk.cols, chk.rows, chk.square_width);
	std::vector<vec2> image_points = checkerboard_image_corners(chk);

	vec3 rotation_vec, translation;
	mat33 rotation;	
	const bool use_ransac = false;
	if(use_ransac) {
		std::vector<cv::Vec3f> object_points_(chk.corners_count());
		std::vector<cv::Vec2f> image_points_(chk.corners_count());
		for(int idx = 0; idx < chk.corners_count(); ++idx) {
			object_points_[idx] = cv::Vec3f( object_points[idx][0], object_points[idx][1], object_points[idx][2] );
			image_points_[idx] = cv::Vec2f( image_points[idx][0], image_points[idx][1] );
		}
				
		cv::solvePnPRansac(
			object_points_,
			image_points_,
			intr.K,
			intr.distortion.cv_coeffs(),
			rotation_vec,
			translation,
			false,
			300,
			0.05,
			chk.corners_count(),
			cv::noArray(),
			cv::ITERATIVE
		);
	} else {
		cv::solvePnP(
			object_points,
			image_points,
			intr.K,
			intr.distortion.cv_coeffs(),
			rotation_vec,
			translation,
			false
		);
	}
	cv::Rodrigues(rotation_vec, rotation);

	return checkerboard_extrinsics {
		translation,
		rotation,
		rotation_vec
	};
}


real checkerboard_reprojection_error(const checkerboard& chk, const intrinsics& intr, const checkerboard_extrinsics& ext) {
	std::vector<vec3> object_points = checkerboard_world_corners(chk.cols, chk.rows, chk.square_width);
	std::vector<vec2> image_points = checkerboard_image_corners(chk);

	std::vector<vec2> reprojected_image_points;
	cv::projectPoints(
		object_points,
		ext.rotation_vec,
		ext.translation,
		intr.K,
		intr.distortion.cv_coeffs(),
		reprojected_image_points
	);
	real err = 0.0;
	for(int idx = 0; idx < chk.corners_count(); ++idx) {
		const vec2& i_orig = image_points[idx];
		const vec2& i_reproj = reprojected_image_points[idx];				
		vec2 diff = i_reproj - i_orig;
		err += sq(diff[0]) + sq(diff[1]);
	}
	err /= chk.corners_count();
	return std::sqrt(err);
}


vec2 checkerboard_parallel_measures(const checkerboard& chk) {
	auto direction_deviation = [](const std::vector<vec2>& vecs) {
		std::vector<vec2> nvecs;
		for(const vec2& vec : vecs)
			nvecs.push_back(vec / std::sqrt(sq(vec[0]) + sq(vec[1])));

		vec2 mean(0.0, 0.0);
		for(const vec2& nvec : nvecs) mean += nvec;
		mean *= 1.0/vecs.size();
		
		real rms = 0.0;
		for(const vec2 nvec : nvecs) {
			vec2 diff = nvec - mean;
			rms += sq(diff[0]) + sq(diff[1]);
		}
		rms /= vecs.size();
		return std::sqrt(rms);
	};
	
	std::vector<vec2> horizontal_vectors, vertical_vectors;
	for(int row = 0; row < chk.rows; ++row)
		horizontal_vectors.push_back(chk.corner(chk.cols-1, row) - chk.corner(0, row));

	for(int col = 0; col < chk.cols; ++col)
		vertical_vectors.push_back(chk.corner(col, chk.rows-1) - chk.corner(col, 0));
		
	return vec2(
		direction_deviation(horizontal_vectors),
		direction_deviation(vertical_vectors)
	);
}


real calculate_parallel_checkerboard_depth(const checkerboard& chk, const intrinsics& intr) {
	// not taking distortion into account
	
	real fx = intr.K(0, 0), fy = intr.K(1, 1);
	
	real w = (chk.cols - 1) * chk.square_width;
	real h = (chk.rows - 1) * chk.square_width;
	
	real iw = 0.0;
	for(int row = 0; row < chk.rows; ++row) {
		vec2 diff = chk.corner(chk.cols-1, row) - chk.corner(0, row);
		iw += sq(diff[0]) + sq(diff[1]);
	}
	iw = std::sqrt(iw / chk.rows);
	
	real ih = 0.0;
	for(int col = 0; col < chk.cols; ++col) {
		vec2 diff = chk.corner(col, chk.rows-1) - chk.corner(col, 0);
		ih += sq(diff[0]) + sq(diff[1]);
	}
	ih = std::sqrt(ih / chk.cols);
	
	real dx = fx * w / iw;
	real dy = fy * h / ih;
	
	//std::cout << "dx: " << dx << "\ndy: " << dy << "\n\n\n";
	
	return (dx + dy) / 2.0;
}


	
std::vector<checkerboard_pixel_depth_sample> checkerboard_pixel_depth_samples(const checkerboard& chk, const cv::Mat_<float>& depth_image, int granularity) {
	// region of interest: bounding box + mask
	cv::Rect bounding_rect = chk.bounding_rect;
	cv::Mat_<uchar> mask(depth_image.size());
	mask.setTo(0);
	std::vector<std::vector<cv::Point>> polylines { chk.outer_corners_i() };
	cv::fillConvexPoly(mask, chk.outer_corners_i().data(), 4, 255);

	// get measured depths for each pixel
	std::vector<checkerboard_pixel_depth_sample> depth_samples;
	for(int ry = 0; ry < bounding_rect.height; ry += granularity)
	for(int rx = 0; rx < bounding_rect.width; rx += granularity) {
		int x = bounding_rect.x + rx, y = bounding_rect.y + ry;
		
		if(! mask(y, x)) continue;
		real d = depth_image(y, x);
		if(d <= 100.0) continue;
		
		checkerboard_pixel_depth_sample samp;
		samp.coordinates = vec2(x, y);
		samp.measured_depth = d;
		samp.calculated_depth = NAN;
		depth_samples.push_back(samp);
	}

	return depth_samples;
}



void calculate_checkerboard_pixel_depths(const intrinsics& intr, const checkerboard_extrinsics& ext, std::vector<checkerboard_pixel_depth_sample>& inout_samples) {
	if(inout_samples.size() == 0) return;

	// get image points
	std::vector<vec2> image_points;
	for(const checkerboard_pixel_depth_sample& sample : inout_samples)
		image_points.push_back(sample.coordinates);
	
	// normal vector and distance of checkerboard in camera view space
	vec3 normal(0.0, 0.0, 1.0);
	normal = ext.rotation * normal;
	real plane_distance = normal.dot(ext.translation);
		
	// undistort image points, and get normalized view space points with depth 1.0
	std::vector<vec2> undistorted_normalized_points;
	cv::undistortPoints(
		image_points,
		undistorted_normalized_points,
		intr.K,
		intr.distortion.cv_coeffs(),
		cv::noArray(),
		cv::noArray()
	);

	// calculate distances
	for(int idx = 0; idx < image_points.size(); ++idx) {		
		vec3 v;
		v[0] = undistorted_normalized_points[idx][0];
		v[1] = undistorted_normalized_points[idx][1];
		v[2] = 1.0;
		
		real t = plane_distance / normal.dot(v);
		v = v * t;
		
		inout_samples[idx].calculated_depth = v[2];
	}
}

}

#endif
