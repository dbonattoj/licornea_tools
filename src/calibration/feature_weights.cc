#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/random_color.h"
#include "../lib/misc.h"
#include "lib/image_correspondence.h"
#include "lib/cg/feature_points.h"
#include <vector>
#include <functional>
#include <cmath>
#include <climits>
#include <string>
#include <map>
#include <algorithm>

using namespace tlz;

constexpr real pi = 3.14159265359;

const std::string window_name = "Feature Clusters";

const cv::Vec3b point_color(0, 200, 0);

std::function<void()> update_function;
void update_callback(int = 0, void* = nullptr) {	
	update_function();
}


std::map<std::string, real> compute_feature_point_weights(
	const std::map<std::string, vec2>& pts,
	int rad_pieces,
	int arg_pieces
) {
	std::size_t points_count = pts.size();
	
	// get center of gravity and bounding box
	vec2 center(0.0, 0.0);
	for(const auto& kv : pts) center += kv.second;
	center = center / (real)points_count;

	real x_min = +INFINITY, x_max = -INFINITY, y_min = +INFINITY, y_max = -INFINITY;
	for(const auto& kv : pts) {
		const vec2& pt = kv.second;
		if(pt[0] > x_max) x_max = pt[0];
		if(pt[0] < x_min) x_min = pt[0];
		if(pt[1] > y_max) y_max = pt[1];
		if(pt[1] < y_min) y_min = pt[1];
		center += pt;
	}
	center = center / (real)pts.size();

	// get max distance from center to corner of bounding box
	auto center_dist_sq = [&center](const vec2& pt) {
		return sq(center[0] - pt[0]) + sq(center[1] - pt[1]);
	};
	const real max_center_dist = std::sqrt(std::max({
		center_dist_sq(vec2(x_min, y_min)),
		center_dist_sq(vec2(x_max, y_min)),
		center_dist_sq(vec2(x_min, y_max)),
		center_dist_sq(vec2(x_max, y_max))
	}));
	

	// max rad and arg of points in polar coordinates (with center at origin)
	const real rad_max = max_center_dist;
	const real arg_max = 2.0 * pi;

	// assign points to pieces
	using piece_idx_type = int;
	int pieces_count = rad_pieces * arg_pieces;	
	std::map<std::string, piece_idx_type> fpoint_pieces;
	std::vector<std::size_t> piece_point_counts(pieces_count, 0);
	
	auto get_piece_idx = [rad_pieces, arg_pieces](int rad_piece, int arg_piece) -> piece_idx_type {
		return rad_pieces*arg_piece + rad_piece;
	};

	for(const auto& kv : pts) {
		const std::string& feature_name = kv.first;
		const vec2& pt = kv.second;
		vec2 d_pt = pt - center;
		real rad = std::sqrt(sq(d_pt[0]) + sq(d_pt[0])); // 0..rad_max
		real arg = std::atan2(d_pt[1], d_pt[0]) + pi; // 0..arg_max
		
		int rad_piece = (rad / rad_max) * rad_pieces;
		int arg_piece = (arg / arg_max) * arg_pieces;
		if(rad_piece >= rad_pieces) rad_piece = rad_pieces - 1;
		if(arg_piece >= arg_pieces) arg_piece = arg_pieces - 1;
		
		piece_idx_type piece_idx = get_piece_idx(rad_piece, arg_piece);
		fpoint_pieces[feature_name] = piece_idx;
		piece_point_counts.at(piece_idx)++;
	}

	// point weights
	std::map<std::string, real> point_weights;
	real weights_sum = 0.0;

	for(const auto& kv : pts) {
		const std::string& feature_name = kv.first;
		
		piece_idx_type piece_idx = fpoint_pieces.at(feature_name);
		real piece_points_count = piece_point_counts.at(piece_idx);
		real ideal_piece_points_count = (real)points_count / pieces_count;
		
		real weight = ideal_piece_points_count / piece_points_count;

		point_weights[feature_name] = weight;
		weights_sum += weight;
	}
	
	// normalize
	for(auto& kv : point_weights) kv.second = (kv.second / weights_sum);

	return point_weights;
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json out_cors.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	
	std::string dataset_group = cors.dataset_group;
	
	int slider_x = cors.reference.x - datas.x_min();
	int slider_y = cors.reference.y - datas.y_min();
	int rad_pieces = 3;
	int arg_pieces = 8;
	int max_dot_rad = 10;
	int image_opacity = 100;
	
	auto update = [&]() {
		int x = slider_x + datas.x_min();
		int y = slider_y + datas.y_min();
		
		if(!datas.x_valid(x) || !datas.y_valid(y)) return;
		if(rad_pieces < 1 || arg_pieces < 1) return;
		view_index idx(x, y);

		cv::Mat_<cv::Vec3b> shown_img;
		{
			std::string image_filename = datas.view(idx).group_view(dataset_group).image_filename();
			cv::Mat_<uchar> img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(img.empty()) return;
			cv::cvtColor(img, shown_img, CV_GRAY2BGR);
		}
		if(image_opacity < 100) shown_img = shown_img * (image_opacity / 100.0);
				

		feature_points fpoints = feature_points_for_view(cors, idx);
		std::map<std::string, real> point_weights = compute_feature_point_weights(fpoints.points, rad_pieces, arg_pieces);
		
		for(const auto& kv : fpoints.points) {
			const std::string& feature_name = kv.first;
			const vec2& pt = kv.second;
			real nweight = point_weights.at(feature_name);
			real area = nweight * fpoints.points.size();
			real rad = std::sqrt(area / pi);
			//cv::Vec3b piece_color = random_color(fpoint_pieces.at(feature_name));
			cv::circle(shown_img, vec2_to_point(pt), max_dot_rad * rad, cv::Scalar(point_color), -1);
		}	
	
		cv::imshow(window_name, shown_img);
	};
	update_function = update;

	cv::namedWindow(window_name, CV_WINDOW_NORMAL);

	cv::createTrackbar("x", window_name, &slider_x, datas.x_max() - datas.x_min(), &update_callback);
	cv::createTrackbar("y", window_name, &slider_y, datas.y_max() - datas.y_min(), &update_callback);
	cv::createTrackbar("rad pieces", window_name, &rad_pieces, 10, &update_callback);
	cv::createTrackbar("arg pieces", window_name, &arg_pieces, 16, &update_callback);
	cv::createTrackbar("dot size", window_name, &max_dot_rad, 100, &update_callback);
	cv::createTrackbar("image", window_name, &image_opacity, 100, &update_callback);

	update();
	
	int key;
	do {
		key = cv::waitKey(0);
	} while(key != escape_keycode && key != enter_keycode);
	
	if(key == enter_keycode) {
		std::cout << "computing feature weights for all views" << std::endl;
		// ...TODO
	}
}
