#include <opencv2/opencv.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/image_correspondence.h"
#include "../lib/json.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: feature_depth_estimate dataset_parameters.json in_image_correspondences.json out_image_correspondences.json\n";
	std::cout << std::endl;
	std::exit(1);
}

float determine_feature_depth(const image_correspondence_feature& feature, const dataset& datas) {
	std::vector<float> depths;
	for(auto kv : feature.points) {
		view_index view_idx = kv.first;
		Eigen_vec2 color_pos = kv.second;
				
		std::string depth_filename = datas.view(view_idx).depth_filename();
		std::cout << "depth file " << depth_filename << std::endl;
		
		cv::Mat_<ushort> depth = load_depth(depth_filename);
		
		ushort depth_value = depth(color_pos[1], color_pos[0]);
		if(depth_value != 0) {
			depths.push_back(depth_value);
			std::cout << "feature " << &feature << "  -->  " << depth_value << std::endl;
		}
	}
}


int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string in_cors_filename = argv[2];
	std::string out_cors_filename = argv[3];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading correspondences" << std::endl;
	json j_features = import_json_file(in_cors_filename);
	for(json::iterator it = j_features.begin(); it != j_features.end(); ++it) {
		std::cout << "feature " << it.key() << std::endl;
		image_correspondence_feature feature = decode_image_correspondence_feature(it.value());
		determine_feature_depth(feature, datas);
	}
	
	std::cout << "done" << std::endl;
}

