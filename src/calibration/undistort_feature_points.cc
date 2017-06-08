#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "lib/feature_points.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_fpoints.json out_fpoints.json intrinsics.json");
	feature_points in_fpoints = feature_points_arg();
	std::string out_fpoints_filename = out_filename_arg();
	intrinsics intr = intrinsics_arg();
		
	feature_points out_fpoints = undistort(in_fpoints, intr);
	
	export_json_file(encode_feature_points(out_fpoints), out_fpoints_filename);
}

