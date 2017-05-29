#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"

using namespace tlz;


static std::string make_feature_name(int i) {
	return "pt" + std::to_string(i);
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json out_image_correspondences.json [out_xy.txt]");
	dataset datas = dataset_arg();
	std::string out_cors_filename = out_filename_arg();
	std::string out_xy_filename = out_filename_opt_arg();

	std::cout << "loading data set" << std::endl;
	if(datas.is_2d()) {
		std::cout << "no support for 2d dataset" << std::endl;
		std::exit(1);
	}
	
	int count_x = datas.x_count(), mid_x = datas.x_mid();
	std::cout << "reference: mid_x=" << mid_x << std::endl;
		
	std::cout << "loading center image" << std::endl;	
	cv::Mat_<cv::Vec3b> center_col_img = cv::imread(datas.view(mid_x).image_filename(), CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_col_img, center_gray_img, CV_BGR2GRAY);
		
	std::cout << "finding good features" << std::endl;
	const std::size_t features_count = 10;
	std::vector<cv::Point2f> center_positions(features_count);
	cv::goodFeaturesToTrack(center_gray_img, center_positions, features_count, 0.3, 7);
	std::cout << "requested " << features_count << " features, found " << center_positions.size() << std::endl;
		
	cv::Mat_<uchar> img;
	std::vector<cv::Point2f> positions;
	std::vector<uchar> status;
	std::vector<image_correspondence_feature> correspondences(features_count);

	auto flow_to = [&img, &positions, &status, &datas](int x) {
		std::vector<cv::Point2f> new_positions(features_count);
		cv::Mat_<cv::Vec3b> new_col_img = cv::imread(datas.view(x).image_filename(), CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> new_img;
		cv::cvtColor(new_col_img, new_img, CV_BGR2GRAY);

		std::vector<uchar> new_status(features_count);
		std::vector<float> err(features_count);
		cv::calcOpticalFlowPyrLK(img, new_img, positions, new_positions, new_status, err);
		for(std::ptrdiff_t feature = 0; feature < features_count; ++feature)
			if(!status[feature] || !new_status[feature]) new_positions[feature] = cv::Point2f(0.0, 0.0);

		img = new_img;
		positions = new_positions;
		status = new_status;
	};
		
	auto add_correspondences = [&correspondences, &positions, features_count](int x) {
		view_index idx(x);
		for(std::ptrdiff_t feature = 0; feature < features_count; ++feature)
			if(positions[feature].x != 0 && positions[feature].y != 0)
				correspondences[feature].points[idx] = vec2(positions[feature].x, positions[feature].y);
	};
	
	std::cout << "optical flow by increasing x starting at mid_x..." << std::endl;

	img = center_gray_img;
	positions = center_positions;
	status.assign(features_count, 1);
	
	add_correspondences(mid_x);
	
	for(int x = mid_x + datas.x_step(); x <= datas.x_max(); x += datas.x_step()) {
		std::cout << '.' << std::flush;
		//std::cout << "   x=" << (x - datas.x_step()) << " --> x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
	}
	
	
	std::cout << "\noptical flow by decreasing x starting at mid_x..." << std::endl;
	std::vector<std::vector<cv::Point2f>> x_decr_frames;

	img = center_gray_img;
	positions = center_positions;
	status.assign(features_count, 1);
	for(int x = mid_x - datas.x_step(); x >= datas.x_min(); x -= datas.x_step()) {
		std::cout << '.' << std::flush;
		//std::cout << "   x=" << (x + datas.x_step()) << " <-- x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
	}
	

	std::cout << "\nsaving image correspondences" << std::endl;
	image_correspondences cors;
	for(std::ptrdiff_t feature = 0; feature < features_count; ++feature) {
		if(correspondences[feature].points.size() != count_x) continue;
		std::string feature_name = make_feature_name(feature);
		cors.features[feature_name] = correspondences[feature];
	}
	std::cout << "retained " << cors.features.size() << " of " << center_positions.size() << " features" << std::endl;
	cors.reference = view_index(mid_x);
	export_json_file(encode_image_correspondences(cors), out_cors_filename);
	
	if(! out_xy_filename.empty()) {
		std::cout << "saving all xy positions" << std::endl;
		std::ofstream stream(out_xy_filename);
		for(const auto& kv : cors.features) {
			const std::string& feature_name = kv.first;
			const auto& points = kv.second.points;
			
			stream << feature_name << "_x";
			for(const auto& kv2 : points) stream << ' ' << kv2.second[0];
			stream << '\n' << feature_name << "_y";
			for(const auto& kv2 : points) stream << ' ' << kv2.second[1];
			stream << '\n';
		}
	}

	std::cout << "done" << std::endl;
}

