#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "lib/image_correspondence.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cors.json out_cors.json intrinsics.json");
	image_correspondences in_cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	intrinsics intr = intrinsics_arg();
		
	image_correspondences out_cors = undistort(in_cors, intr);
	
	export_json_file(encode_image_correspondences(out_cors), out_cors_filename);
}

