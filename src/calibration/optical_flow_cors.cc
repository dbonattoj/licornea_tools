#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include <mutex>
#include <cmath>
#include <map>
#include "lib/feature_points.h"
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/image_io.h"

using namespace tlz;

const bool verbose = false;


struct flow_state {
	view_index view_idx;
	cv::Mat_<uchar> image;
	std::vector<cv::Point2f> feature_positions;
	std::vector<uchar> feature_status;
	
	flow_state() = default;
	flow_state(const view_index& idx, const cv::Mat_<uchar>& img, std::size_t features_count) :
		view_idx(idx),
		image(img),
		feature_positions(features_count),
		feature_status(features_count) { }
		
	std::size_t features_count() const { return feature_positions.size(); }
};


int max_features_count;
int horizontal_key;
int horizontal_outreach;
int vertical_key;
int vertical_outreach;
std::string dataset_group;

using local_feature_index = std::ptrdiff_t;
using correspondence_key_type = std::pair<local_feature_index, view_index>;
using correspondences_type = std::map<correspondence_key_type, vec2>;

inline correspondence_key_type cor_key(local_feature_index i, const view_index& idx) {
	return std::make_pair(i, idx);
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


void add_correspondences(correspondences_type& cors, const flow_state& state) {
	std::size_t count = state.feature_positions.size();
	for(local_feature_index i = 0; i < count; ++i) {
		if(state.feature_status[i]) {
			vec2 pos = point2f_to_vec2(state.feature_positions[i]);
			cors[std::make_pair(i, state.view_idx)] = pos;
		}
	}
}


cv::Mat_<uchar> load_image(const dataset& datas, const view_index& idx) {
	std::string image_filename = datas.view(idx).group_view(dataset_group).image_filename();
	cv::Mat_<cv::Vec3b> col_img = load_texture(image_filename);
	cv::Mat_<uchar> gray_img;
	cv::cvtColor(col_img, gray_img, CV_BGR2GRAY);
	return gray_img;
}



flow_state flow_to(const flow_state& origin_state, view_index dest_idx, const dataset& datas) {
	cv::Mat_<uchar> dest_img = load_image(datas, dest_idx);
	
	std::size_t features_count = origin_state.features_count();
	
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


void do_horizontal_optical_flow(correspondences_type& cors, const view_index& reference_idx, const flow_state& mid_x_state, const dataset& datas, bool verb = false) {
	int x_min = std::max(datas.x_min(), reference_idx.x - horizontal_outreach);
	int x_max = std::min(datas.x_max(), reference_idx.x + horizontal_outreach);

	if(verb) std::cout << "horizontal optical flow by increasing x starting at mid_x..." << std::endl;
	flow_state state = mid_x_state;
	for(int x = mid_x_state.view_idx.x + datas.x_step(); x <= x_max; x += datas.x_step()) {
		view_index idx(x, mid_x_state.view_idx.y);
		print_flow_indicator(state.view_idx, idx);
		flow_state new_state = flow_to(state, idx, datas);
		add_correspondences(cors, new_state);
		state = std::move(new_state);
	}
	
	if(verb) std::cout << "\nhorizontal optical flow by decreasing x starting at mid_x..." << std::endl;
	state = mid_x_state;
	for(int x = mid_x_state.view_idx.x - datas.x_step(); x >= x_min; x -= datas.x_step()) {
		view_index idx(x, mid_x_state.view_idx.y);
		print_flow_indicator(state.view_idx, idx);
		flow_state new_state = flow_to(state, idx, datas);
		add_correspondences(cors, new_state);
		state = std::move(new_state);
	}
}


correspondences_type do_2d_optical_flow(const dataset& datas, const view_index& reference_idx, std::size_t max_wanted_features) {
	std::cout << "doing optical flow from reference view " << reference_idx << std::endl;

	int y_min = std::max(datas.y_min(), reference_idx.y - vertical_outreach);
	int y_max = std::min(datas.y_max(), reference_idx.y + vertical_outreach);

	cv::Mat_<uchar> center_image = load_image(datas, reference_idx);
	
	std::vector<cv::Point2f> center_positions;
	cv::goodFeaturesToTrack(center_image, center_positions, max_wanted_features, 0.3, 7);
	std::size_t features_count = center_positions.size();

	flow_state center_state(reference_idx, center_image, features_count);
	center_state.feature_positions = center_positions;
	center_state.feature_status.assign(features_count, 1);
	
	correspondences_type cors;
	add_correspondences(cors, center_state);
	
	
	if(datas.is_2d()) {
		// 2D MODE
		std::vector<flow_state> vertical_origins;		
		vertical_origins.push_back(center_state);
		
		std::cout << "vertical optical flow by increasing y starting at mid_y..." << std::endl;
		flow_state state = center_state;
		for(int y = reference_idx.y + datas.y_step(); y <= y_max; y += datas.y_step()) {
			view_index idx(reference_idx.x, y);
			print_flow_indicator(state.view_idx, idx);
			flow_state new_state = flow_to(state, idx, datas);
			add_correspondences(cors, new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
		}
		
		
		std::cout << "\nvertical optical flow by decreasing y starting at mid_y..." << std::endl;
		state = center_state;	
		for(int y = reference_idx.y - datas.y_step(); y >= y_min; y -= datas.y_step()) {
			view_index idx(reference_idx.x, y);
			print_flow_indicator(state.view_idx, idx);
			flow_state new_state = flow_to(state, idx, datas);
			add_correspondences(cors, new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
		}

		std::cout << "\nnow doing horizontal flows..." << std::endl;
		int done = 0;
		#pragma omp parallel for ordered num_threads(3)
		for(std::ptrdiff_t i = 0; i < vertical_origins.size(); i++) {
			correspondences_type hcors;
			
			const flow_state& origin_state = vertical_origins[i];
			do_horizontal_optical_flow(hcors, reference_idx, origin_state, datas);
									
			#pragma omp ordered
			{
				++done;
				std::cout << '\n' << done << " of " << vertical_origins.size() << std::endl;
				
				cors.insert(hcors.begin(), hcors.end());
			}
		}


	} else {
		// 1D MODE
	
		do_horizontal_optical_flow(cors, reference_idx, center_state, datas, true);
	}

	return cors;
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json out_image_correspondences.json [max_feature_count=100] [horiz_key=80] [horiz_outreach=50] [vert_key=80] [vert_outreach=50] [dataset_group]");
	dataset datas = dataset_arg();
	std::string out_cors_filename = out_filename_arg();
	max_features_count = int_opt_arg(100);
	horizontal_key = int_opt_arg(80);
	horizontal_outreach = int_opt_arg(50);
	vertical_key = int_opt_arg(80);
	vertical_outreach = int_opt_arg(50);
	dataset_group = string_opt_arg("");


	image_correspondences out_cors;
	out_cors.dataset_group = dataset_group;
	int global_feature_counter = 0;

	auto add_optical_flow = [&](int ref_x, int ref_y) {
		view_index reference_view_idx(ref_x, ref_y);
		correspondences_type cors = do_2d_optical_flow(datas, reference_view_idx, max_features_count);
		
		std::map<local_feature_index, std::string> feature_names;
		
		for(const auto& kv : cors) {
			const local_feature_index& local_feature_idx = kv.first.first;
			const view_index& idx = kv.first.second;
			const vec2& position = kv.second;
			
			std::string& feature_name = feature_names[local_feature_idx];
			if(feature_name.empty()) feature_name = "feat" + std::to_string(global_feature_counter++);
			
			image_correspondence_feature& feature = out_cors.features[feature_name];
			feature.reference_view = reference_view_idx;
			feature.points[idx].position = position;
		}
	};
	
	view_index center_idx(datas.x_mid(), datas.y_mid());
	
	add_optical_flow(center_idx.x, center_idx.y);

	for(int y = center_idx.y + vertical_key; y <= datas.y_max(); y += vertical_key) {
		for(int x = center_idx.x + horizontal_key; x <= datas.x_max(); x += horizontal_key) add_optical_flow(x, y);
		for(int x = center_idx.x - horizontal_key; x >= datas.x_min(); x -= horizontal_key) add_optical_flow(x, y);
	}
	for(int y = center_idx.y - vertical_key; y >= datas.y_min(); y -= vertical_key) {
		for(int x = center_idx.x + horizontal_key; x <= datas.x_max(); x += horizontal_key) add_optical_flow(x, y);
		for(int x = center_idx.x - horizontal_key; x >= datas.x_min(); x -= horizontal_key) add_optical_flow(x, y);		
	}
	

	std::cout << "\nsaving image correspondences" << std::endl;
	export_json_file(encode_image_correspondences(out_cors), out_cors_filename);
	
	std::cout << "done" << std::endl;
}

