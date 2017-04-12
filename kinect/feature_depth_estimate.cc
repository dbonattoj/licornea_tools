#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/image_correspondence.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: feature_depth_estimate dataset_parameters.json in_image_correspondences.json intrinsics.json out_image_correspondences.json\n";
	std::cout << std::endl;
	std::exit(1);
}


int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string in_cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cors_filename = argv[3];
	
	
}

