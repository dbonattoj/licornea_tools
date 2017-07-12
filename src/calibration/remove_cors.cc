#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/string.h"
#include "lib/image_correspondence.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cors.json out_cors.json removed_feat1,removed,feat2,...");
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	std::string removed_cors_str = string_arg();

	std::vector<std::string> removed_cors = explode(',', removed_cors_str);
	for(const std::string& removed_feature_name : removed_cors) {
		cors.features.erase(removed_feature_name);
	}

	export_image_corresponcences(cors, out_cors_filename);
}

