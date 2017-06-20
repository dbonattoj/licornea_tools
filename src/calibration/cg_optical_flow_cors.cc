#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include <mutex>
#include <cmath>
#include <map>
#include <format.h>
#include "lib/feature_points.h"
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/image_io.h"

using namespace tlz;

const bool verbose = false;
const bool small_subset_mode = false;

const real max_flow_err = 4.0;
const real min_distance_between_features = 60;
const real min_distance_to_existing_features = 100;
const real features_quality_level = 0.5;
const real features_from_existing_pieces_amount = 0.7;
const cv::Size horizontal_optical_flow_window_size(20, 20);
const cv::Size vertical_optical_flow_window_size(20, 20);
const int max_pyramid_level = 3;

struct flow_state {
	view_index view_idx;
	std::vector<cv::Mat_<uchar>> image_pyramid;
	std::vector<cv::Point2f> feature_positions;
	std::vector<uchar> feature_status;
	
	flow_state() = default;
	flow_state(const view_index& idx, const cv::Mat_<uchar>& img, std::size_t features_count) :
		view_idx(idx),
		image_pyramid({img}),
		feature_positions(features_count),
		feature_status(features_count) { }
			
	std::size_t features_count() const { return feature_positions.size(); }
	
	std::size_t valid_features_count() const;
};

std::size_t flow_state::valid_features_count() const {
	std::size_t count = 0;
	for(uchar status : feature_status) if(status) ++count;
	return count;
}

int max_features_count;
int horizontal_key;
int horizontal_outreach;
int vertical_key;
int vertical_outreach;

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


cv::Mat_<uchar> load_image(const dataset_group& datag, const view_index& idx) {
	static std::mutex disk_read_lock;
	//std::lock_guard<std::mutex> lock(disk_read_lock);
	std::string image_filename = datag.view(idx).image_filename();
	cv::Mat_<cv::Vec3b> col_img = load_texture(image_filename);
	cv::Mat_<uchar> gray_img;
	cv::cvtColor(col_img, gray_img, CV_BGR2GRAY);
	return gray_img;
}



flow_state flow_to(flow_state& origin_state, view_index dest_idx, const dataset_group& datag, const cv::Size& window_size) {
	std::size_t features_count = origin_state.features_count();

	if(origin_state.image_pyramid.size() <= max_pyramid_level) {
		cv::Mat_<uchar> orig_img;
		origin_state.image_pyramid.front().copyTo(orig_img);
		origin_state.image_pyramid.clear();
		cv::buildOpticalFlowPyramid(
			orig_img,
			origin_state.image_pyramid,
			window_size,
			max_pyramid_level
		);
	}

	cv::Mat_<uchar> dest_img = load_image(datag, dest_idx);
	flow_state dest_state(dest_idx, dest_img, features_count);
	if(max_pyramid_level > 0) {
		dest_state.image_pyramid.clear();
		cv::buildOpticalFlowPyramid(
			dest_img,
			dest_state.image_pyramid,
			window_size,
			max_pyramid_level
		);
	}

	std::vector<cv::Point2f>& dest_positions = dest_state.feature_positions;
	std::vector<uchar>& dest_status = dest_state.feature_status;
	std::vector<float> dest_errs(features_count);
	
	cv::TermCriteria term(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);
	cv::calcOpticalFlowPyrLK(
		origin_state.image_pyramid,
		dest_state.image_pyramid,
		origin_state.feature_positions,
		dest_positions,
		dest_status,
		dest_errs,
		window_size,
		max_pyramid_level,
		term
	);
	

	auto position_ok = [&](const cv::Point2f& pos) -> bool {
		return (pos.x > 0.0) && (pos.y > 0.0) && (pos.x < dest_img.cols) && (pos.y < dest_img.rows);
	};

	for(std::ptrdiff_t feature = 0; feature < features_count; ++feature) {	
		bool status = origin_state.feature_status[feature]
			&& dest_status[feature]
			&& position_ok(dest_positions[feature])
			&& dest_positions[feature] != origin_state.feature_positions[feature]
			&& dest_errs[feature] <= max_flow_err;
		dest_status[feature] = status;
	}

	return dest_state;
}


void do_horizontal_optical_flow(correspondences_type& cors, const view_index& reference_idx, const flow_state& mid_x_state, const dataset_group& datag, bool verb = false) {
	const dataset& datas = datag.set();
	int x_min = std::max(datas.x_min(), reference_idx.x - horizontal_outreach);
	int x_max = std::min(datas.x_max(), reference_idx.x + horizontal_outreach);

	flow_state state = mid_x_state;
	state.image_pyramid = { state.image_pyramid.front() }; // let flow_to() recalculate pyramid, because window size if different

	if(verb) std::cout << "horizontal optical flow by increasing x starting at mid_x..." << std::endl;
	for(int x = mid_x_state.view_idx.x + datas.x_step(); x <= x_max; x += datas.x_step()) {
		view_index idx(x, mid_x_state.view_idx.y);
		print_flow_indicator(state.view_idx, idx);
		flow_state new_state = flow_to(state, idx, datag, horizontal_optical_flow_window_size);
		add_correspondences(cors, new_state);
		state = std::move(new_state);
		if(state.valid_features_count() == 0) break;
	}
	
	if(verb) std::cout << "\nhorizontal optical flow by decreasing x starting at mid_x..." << std::endl;
	state = mid_x_state;
	for(int x = mid_x_state.view_idx.x - datas.x_step(); x >= x_min; x -= datas.x_step()) {
		view_index idx(x, mid_x_state.view_idx.y);
		print_flow_indicator(state.view_idx, idx);
		flow_state new_state = flow_to(state, idx, datag, horizontal_optical_flow_window_size);
		add_correspondences(cors, new_state);
		state = std::move(new_state);
		if(state.valid_features_count() == 0) break;
	}
}


std::vector<cv::Point2f> choose_reference_points(const cv::Mat_<uchar>& img, std::size_t max_wanted_features, const std::vector<vec2>& existing_points) {	
	std::cout << "choosing features to track from reference image (wanted max: " << max_wanted_features << ")" << std::endl;

	int width = img.cols, height = img.rows;
	
	std::vector<cv::Point2f> positions;
	cv::Mat_<uchar> far_from_existing_points_mask(height, width);
	far_from_existing_points_mask.setTo(255);
	for(const vec2& pt : existing_points)
		cv::circle(far_from_existing_points_mask, vec2_to_point(pt), min_distance_to_existing_features, 0, -1);

	
	auto get_piece_mask = [width, height](int piece) {
		cv::Mat_<uchar> mask(height, width); mask.setTo(0);
		int w = width/2, h = height/2;
		switch(piece) {
			case 0: mask(cv::Rect(0, 0, w, h)).setTo(255); break; // top-left
			case 1: mask(cv::Rect(w, 0, w, h)).setTo(255); break; // top-right
			case 2: mask(cv::Rect(0, h, w, h)).setTo(255); break; // bottom-left
			case 3: mask(cv::Rect(w, h, w, h)).setTo(255); break; // bottom-right
		}
		return mask;
	};
	const int pieces_count = 4;

	int wanted_new_features = max_wanted_features - existing_points.size()*features_from_existing_pieces_amount;

	for(int piece = 0; piece < pieces_count; ++piece) {
		cv::Mat_<uchar> piece_mask = get_piece_mask(piece);
		int wanted = wanted_new_features / pieces_count;
		cv::Mat_<uchar> mask = piece_mask & far_from_existing_points_mask;
		std::vector<cv::Point2f> piece_positions;
		cv::goodFeaturesToTrack(img, piece_positions, wanted, features_quality_level, min_distance_between_features, mask);
		positions.insert(positions.end(), piece_positions.begin(), piece_positions.end());
	}

	if(existing_points.size() > 0) {
		std::vector<cv::Point2f> near_positions;
		int remaining_wanted_features = max_wanted_features - positions.size();
		cv::Mat_<uchar> near_to_existing_points_mask = (far_from_existing_points_mask == 0);
		cv::goodFeaturesToTrack(img, near_positions, remaining_wanted_features, features_quality_level, min_distance_between_features, near_to_existing_points_mask);
		positions.insert(positions.end(), near_positions.begin(), near_positions.end());
	}
	
	std::cout << "got " << positions.size() << " features" << std::endl;
	return positions;
}


correspondences_type do_2d_optical_flow(const dataset_group& datag, const view_index& reference_idx, std::size_t max_wanted_features, const std::vector<vec2>& existing_points) {
	std::cout << "doing optical flow from reference view " << reference_idx << std::endl;

	const dataset& datas = datag.set();
	int y_min = std::max(datas.y_min(), reference_idx.y - vertical_outreach);
	int y_max = std::min(datas.y_max(), reference_idx.y + vertical_outreach);

	cv::Mat_<uchar> center_image = load_image(datag, reference_idx);
	
	std::vector<cv::Point2f> center_positions = choose_reference_points(center_image, max_wanted_features, existing_points);
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
			flow_state new_state = flow_to(state, idx, datag, vertical_optical_flow_window_size);
			add_correspondences(cors, new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
			if(state.valid_features_count() == 0) break;
		}
		
		
		std::cout << "\nvertical optical flow by decreasing y starting at mid_y..." << std::endl;
		state = center_state;	
		for(int y = reference_idx.y - datas.y_step(); y >= y_min; y -= datas.y_step()) {
			view_index idx(reference_idx.x, y);
			print_flow_indicator(state.view_idx, idx);
			flow_state new_state = flow_to(state, idx, datag, vertical_optical_flow_window_size);
			add_correspondences(cors, new_state);
			vertical_origins.push_back(new_state);
			state = std::move(new_state);
			if(state.valid_features_count() == 0) break;
		}

		std::cout << "\nnow doing horizontal flows..." << std::endl;
		int done = 0;
		#pragma omp parallel for
		for(std::ptrdiff_t i = 0; i < vertical_origins.size(); i++) {
			correspondences_type hcors;
			
			const flow_state& origin_state = vertical_origins[i];
			do_horizontal_optical_flow(hcors, reference_idx, origin_state, datag);
									
			#pragma omp critical
			{
				++done;
				std::cout << '\n' << done << " of " << vertical_origins.size() << std::endl;
				
				cors.insert(hcors.begin(), hcors.end());
			}
		}


	} else {
		// 1D MODE
	
		do_horizontal_optical_flow(cors, reference_idx, center_state, datag, true);
	}

	return cors;
}


std::string make_feature_name(int global_index) {
	return fmt::format("feat{:04d}", global_index);
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
	std::string dataset_group_name = string_opt_arg("");

	if(small_subset_mode) {
		horizontal_key = datas.x_count();
		horizontal_outreach = 100 * datas.x_step();
		vertical_key = datas.x_count();
		vertical_outreach = 10 * datas.y_step();
	}

	dataset_group datag = datas.group(dataset_group_name);

	image_correspondences out_cors;
	out_cors.dataset_group = dataset_group_name;
	int global_feature_counter = 0;

	cv::setNumThreads(0);

	auto add_optical_flow = [&](int ref_x, int ref_y) {		
		view_index reference_view_idx(ref_x, ref_y);

		std::vector<vec2> existing_feature_points = positions(feature_points_for_view(out_cors, reference_view_idx));

		correspondences_type cors = do_2d_optical_flow(datag, reference_view_idx, max_features_count, existing_feature_points);
		
		std::map<local_feature_index, std::string> feature_names;
		
		for(const auto& kv : cors) {
			const local_feature_index& local_feature_idx = kv.first.first;
			const view_index& idx = kv.first.second;
			const vec2& position = kv.second;
			
			std::string& feature_name = feature_names[local_feature_idx];
			if(feature_name.empty()) feature_name = make_feature_name(global_feature_counter++);
			
			image_correspondence_feature& feature = out_cors.features[feature_name];
			feature.reference_view = reference_view_idx;
			feature.points[idx].position = position;
		}
	};
	
	view_index center_idx(datas.x_mid(), datas.y_mid());
	
	add_optical_flow(center_idx.x, center_idx.y);
	for(int x = center_idx.x + horizontal_key; x <= datas.x_max(); x += horizontal_key) add_optical_flow(x, center_idx.y);
	for(int x = center_idx.x - horizontal_key; x >= datas.x_min(); x -= horizontal_key) add_optical_flow(x, center_idx.y);

	for(int y = center_idx.y + vertical_key; y <= datas.y_max(); y += vertical_key) {
		for(int x = center_idx.x + horizontal_key; x <= datas.x_max(); x += horizontal_key) add_optical_flow(x, y);
		for(int x = center_idx.x - horizontal_key; x >= datas.x_min(); x -= horizontal_key) add_optical_flow(x, y);
	}
	for(int y = center_idx.y - vertical_key; y >= datas.y_min(); y -= vertical_key) {
		for(int x = center_idx.x + horizontal_key; x <= datas.x_max(); x += horizontal_key) add_optical_flow(x, y);
		for(int x = center_idx.x - horizontal_key; x >= datas.x_min(); x -= horizontal_key) add_optical_flow(x, y);		
	}

	std::cout << "\nsaving image correspondences" << std::endl;
	export_image_corresponcences(out_cors, out_cors_filename);
	
	std::cout << "done" << std::endl;
}

