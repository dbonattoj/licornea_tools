#include <opencv2/opencv.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include "liv/depth_io.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/image_correspondence.h"
#include "../lib/kinect_intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: feature_depth_estimate dataset_parameters.json in_image_correspondences.json intrinsics.json out_image_correspondences.json\n";
	std::cout << std::endl;
	std::exit(1);
}

float determine_feature_depth(const image_correspondence_feature& feature, const dataset& set, const kinect_intrinsic_parameters& intrinsics) {
	std::vector<float> depths;
	for(auto kv : feature.points) {
		image_correspondence_feature::view_index_type view_idx = kv.first;
		Eigen_vec2 color_pos = kv.second;
		
		std::string depth_filename = set.view(kv.first, kv.second).raw_depth_filename();
		cv::Mat_<ushort> depth = load_depth(depth_filename.c_str());
		
		// doto get depth
	}
}


int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string in_cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cors_filename = argv[3];
	
	std::cout << "loading data set" << std::endl;
	dataset set(dataset_parameter_filename);

	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}


}

