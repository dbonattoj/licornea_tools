#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include <mutex>
#include <cmath>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/image_io.h"

using namespace tlz;

const bool verbose = false;


struct flow_state {
	view_index index;
	cv::Mat_<uchar> image;
	std::vector<cv::Point2f> feature_positions;
	std::vector<uchar> feature_status;
	
	flow_state() = default;
	flow_state(const view_index& ind, const cv::Mat_<uchar>& img, std::size_t features_count) :
		index(ind),
		image(img),
		feature_positions(features_count),
		feature_status(features_count) { }
};


std::vector<image_correspondence_feature> correspondences;
std::mutex correspondences_mutex;
int features_count = 0;
std::string dataset_group;

std::string make_feature_name(int feature) {
	return "feat" + std::to_string(feature);
}


void print_flow_indicator(const view_index& origin_idx, const view_index& dest_idx) {
	char dir = '?';
	if(origin_idx.y < dest_idx.y) dir = '^';
	else if(origin_idx.y > dest_idx.y) dir = 'v';
	else if(origin_idx.x < dest_idx.x) dir = '>';
	else if(origin_idx.x > dest_idx.x) dir = '<';
	
	if(verbose) std::cout << origin_idx << " --" << dir << "-- " << dest_idx << std::endl;
	else std::cout << dir << std::flush;
}


void add_correspondences(const flow_state& state) {
	std::lock_guard<std::mutex> lock(correspondences_mutex);

	for(std::ptrdiff_t feature = 0; feature < features_count; ++feature) {
		if(state.feature_status[feature]) {
			vec2 pt = point2f_to_vec2(state.feature_positions[feature]);
			correspondences[feature].points[state.index] = pt; 
		}
	}
}


flow_state flow_to(const flow_state& origin_state, view_index dest_idx, const dataset& datas) {
	cv::Mat_<cv::Vec3b> dest_col_img = load_texture(datas.view(dest_idx).group_view(dataset_group).image_filename());
	cv::Mat_<uchar> dest_img;
	cv::cvtColor(dest_col_img, dest_img, CV_BGR2GRAY);
	
	flow_state state(dest_idx, dest_img, features_count);

	std::vector<cv::Point2f>& dest_positions = state.feature_positions;
	std::vector<uchar>& dest_status = state.feature_status;
	std::vector<float> err(features_count);
	cv::calcOpticalFlowPyrLK(origin_state.image, dest_img, origin_state.feature_positions, dest_positions, dest_status, err);

	auto position_ok = [&](const cv::Point2f& pos) -> bool {
		return (pos.x > 0.0) && (pos.y > 0.0) && (pos.x < dest_img.cols) && (pos.y < dest_img.rows);
	};

	for(std::ptrdiff_t feature = 0; feature < features_count; ++feature) {
		bool status = origin_state.feature_status[feature]
			&& dest_status[feature]
			&& position_ok(dest_positions[feature]);
		dest_status[feature] = status;
	}

	return state;
}


void do_horizontal_optical_flow(const flow_state& mid_x_state, const dataset& datas, bool verb = false) {
	if(verb) std::cout << "horizontal optical flow by increasing x starting at mid_x..." << std::endl;
	flow_state state = mid_x_state;
	for(int x = mid_x_state.index.x + datas.x_step(); x <= datas.x_max(); x += datas.x_step()) {
		view_index idx(x, mid_x_state.index.y);
		print_flow_indicator(state.index, idx);
		flow_state new_state = flow_to(state, idx, datas);
		add_correspondences(new_state);
		state = std::move(new_state);
	}
	
	if(verb) std::cout << "\nhorizontal optical flow by decreasing x starting at mid_x..." << std::endl;
	state = mid_x_state;
	for(int x = mid_x_state.index.x - datas.x_step(); x >= datas.x_min(); x -= datas.x_step()) {
		view_index idx(x, mid_x_state.index.y);
		print_flow_indicator(state.index, idx);
		flow_state new_state = flow_to(state, idx, datas);
		add_correspondences(new_state);
		state = std::move(new_state);
	}
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json out_image_correspondences.json [feature_count=100] [need_full?] [dataset_group]");
	dataset datas = dataset_arg();
	std::string out_cors_filename = out_filename_arg();
	features_count = int_opt_arg(100);
	bool need_full = bool_opt_arg("need_full");
	dataset_group = string_opt_arg();

	view_index center_idx;
	if(datas.is_1d()) center_idx = view_index(datas.x_mid());
	else center_idx = view_index(datas.x_mid(), datas.y_mid());
	std::cout << "center idx=" << center_idx << std::endl;
	if(need_full) std::cout << "will retain only correspondences found on all images" << std::endl;
	
	std::cout << "loading center image" << std::endl;	
	cv::Mat_<cv::Vec3b> center_col_img = load_texture(datas.view(center_idx).group_view(dataset_group).image_filename());
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_col_img, center_gray_img, CV_BGR2GRAY);
		
	std::cout << "finding good features" << std::endl;
	std::vector<cv::Point2f> center_positions;
	cv::goodFeaturesToTrack(center_gray_img, center_positions, features_count, 0.3, 7);
	std::cout << "requested " << features_count << " features, found " << center_positions.size() << std::endl;
	
	features_count = center_positions.size();

	correspondences.assign(features_count, image_correspondence_feature());

	flow_state center_state;
	center_state.index = center_idx;
	center_state.image = center_gray_img;
	center_state.feature_positions = center_positions;
	center_state.feature_status.assign(center_positions.size(), 1);
	add_correspondences(center_state);
	

	if(datas.is_2d()) {
		std::vector<flow_state> vertical_origins;
		
		vertical_origins.push_back(center_state);
		
		// 2D MODE
		std::cout << "vertical optical flow by increasing y starting at mid_y..." << std::endl;
		flow_state state = center_state;
		for(int y = center_idx.y + datas.y_step(); y <= datas.y_max(); y += datas.y_step()) {
			view_index idx(center_idx.x, y);
			print_flow_indicator(state.index, idx);
			flow_state new_state = flow_to(state, idx, datas);
			add_correspondences(new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
		}
		
		
		std::cout << "\nvertical optical flow by decreasing y starting at mid_y..." << std::endl;
		state = center_state;	
		for(int y = center_idx.y - datas.y_step(); y >= datas.y_min(); y -= datas.y_step()) {
			view_index idx(center_idx.x, y);
			print_flow_indicator(state.index, idx);
			flow_state new_state = flow_to(state, idx, datas);
			add_correspondences(new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
		}

		std::cout << "\nnow doing horizontal flows..." << std::endl;
		int done = 0;
		#pragma omp parallel for num_threads(3) schedule(guided)
		for(std::ptrdiff_t i = 0; i < vertical_origins.size(); i++) {
			const flow_state& origin_state = vertical_origins[i];
			do_horizontal_optical_flow(origin_state, datas);
			
			#pragma omp critical
			{
				++done;
				std::cout << '\n' << done << " of " << vertical_origins.size() << std::endl;
			}
		}


	} else {
		// 1D MODE
	
		do_horizontal_optical_flow(center_state, datas, true);
	}
		
	

	std::cout << "\nsaving image correspondences" << std::endl;
	int expected_points_count = datas.x_count() * datas.y_count();
	image_correspondences cors;
	for(std::ptrdiff_t feature = 0; feature < features_count; ++feature) {		
		if(need_full && correspondences[feature].points.size() != expected_points_count) continue;
		std::string feature_name = make_feature_name(feature);
		cors.features[feature_name] = correspondences[feature];
	}
	std::cout << "retained " << cors.features.size() << " of " << center_positions.size() << " features" << std::endl;
	cors.reference = center_idx;
	cors.dataset_group = dataset_group;
	export_json_file(encode_image_correspondences(cors), out_cors_filename);
	
	std::cout << "done" << std::endl;
}

