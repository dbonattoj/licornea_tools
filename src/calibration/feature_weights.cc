#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/random_color.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <vector>
#include <functional>
#include <cmath>
#include <climits>
#include <string>
#include <map>
#include <algorithm>

using namespace tlz;

const cv::Vec3b point_color(0, 200, 0);



std::map<std::string, real> compute_feature_point_weights(
	const std::map<std::string, feature_point>& pts,
	int rad_pieces,
	int arg_pieces
) {
	std::size_t points_count = pts.size();
	
	// get center of gravity and bounding box
	vec2 center(0.0, 0.0);
	for(const auto& kv : pts) center += kv.second.position;
	center = center / (real)points_count;

	real x_min = +INFINITY, x_max = -INFINITY, y_min = +INFINITY, y_max = -INFINITY;
	for(const auto& kv : pts) {
		const vec2& pt = kv.second.position;
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
	using piece_id_type = std::pair<int, int>;
	int pieces_count = rad_pieces * arg_pieces;	
	std::map<std::string, piece_id_type> fpoint_pieces;
	std::map<piece_id_type, std::size_t> piece_point_counts;

	for(const auto& kv : pts) {
		const std::string& feature_name = kv.first;
		const vec2& pt = kv.second.position;
		vec2 d_pt = pt - center;
		real rad = std::sqrt(sq(d_pt[0]) + sq(d_pt[0])); // 0..rad_max
		real arg = std::atan2(d_pt[1], d_pt[0]) + pi; // 0..arg_max
		
		int rad_piece = (rad / rad_max) * rad_pieces;
		int arg_piece = (arg / arg_max) * arg_pieces;
		if(rad_piece >= rad_pieces) rad_piece = rad_pieces - 1;
		if(arg_piece >= arg_pieces) arg_piece = arg_pieces - 1;
		
		piece_id_type piece_id(rad_piece, arg_piece);
		fpoint_pieces[feature_name] = piece_id;
		piece_point_counts[piece_id]++;
	}

	// point weights
	std::map<std::string, real> point_weights;
	real weights_sum = 0.0;

	const real full_area = pi * sq(rad_max);
	auto get_piece_area = [rad_max, rad_pieces, arg_pieces](piece_id_type id) -> real {
		return pi * sq(rad_max/rad_pieces) * (2*id.first + 1) / arg_pieces;
	};

	for(const auto& kv : pts) {
		const std::string& feature_name = kv.first;
		
		piece_id_type piece_id = fpoint_pieces.at(feature_name);
		real piece_points_count = piece_point_counts.at(piece_id);
		real piece_area = get_piece_area(piece_id);
		
		real ideal_piece_points_count = (piece_area * points_count) / (pieces_count * full_area);
		
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
		
	dataset_group datag = datas.group(cors.dataset_group);
	
	viewer view("Feature Weights");
	auto& x_slider = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& y_slider = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
	auto& rad_pieces_slider = view.add_int_slider("rad pieces", 3, 1, 10);
	auto& arg_pieces_slider = view.add_int_slider("arg_pieces", 1, 1, 16);
	auto& dot_size_slider = view.add_int_slider("dot size", 10, 1, 100);
	auto& image_opacity_slider = view.add_real_slider("image", 0.0, 0.0, 1.0);
	 
	view.update_callback = [&]() {
		int x = x_slider.value(), y = y_slider.value();
		view_index idx(x, y);
		if(! datas.valid(idx))) return;

		cv::Mat_<cv::Vec3b> shown_img;
		{
			std::string image_filename = datas.view(idx).group_view(dataset_group).image_filename();
			cv::Mat_<uchar> img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(img.empty()) return;
			cv::cvtColor(img, shown_img, CV_GRAY2BGR);
		}
		if(image_opacity_slider.value() < 1.0) shown_img = shown_img * image_opacity_slider.value();

		feature_points fpoints = feature_points_for_view(cors, idx);
		std::map<std::string, real> point_weights = compute_feature_point_weights(fpoints.points, rad_pieces, arg_pieces);
		
		for(const auto& kv : fpoints.points) {
			const std::string& feature_name = kv.first;
			const feature_point& pt = kv.second;
			real nweight = point_weights.at(feature_name);
			real area = nweight * fpoints.points.size();
			real rad = std::sqrt(area / pi);
			cv::circle(shown_img, vec2_to_point(pt.position), max_dot_rad * rad, cv::Scalar(point_color), -1);
		}	
	
		view.clear(shown_img.size());
		view.draw(cv::Point(0, 0), shown_img);
	};

	
	int key;
	do {
		key = cv::waitKey(0);
	} while(key != escape_keycode && key != enter_keycode);
	
	if(key == enter_keycode) {
		std::cout << "computing feature weights for all views" << std::endl;
		image_correspondences out_cors = cors;
		//////// TODO
	}
}
