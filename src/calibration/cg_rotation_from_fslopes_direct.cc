#include <string> 
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cassert>
#include "lib/cg/feature_slopes.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/assert.h"

using namespace tlz;

constexpr bool verbose = true;

int main(int argc, const char* argv[]) {
	// unfinished
	
	get_args(argc, argv, "measured_feature_slopes.json intrinsics.json out_rotation.json");
	feature_slopes measured_fslopes = feature_slopes_arg();
	intrinsics intr = intrinsics_arg();
	std::string  out_rotation_filename = out_filename_arg();

	Assert(! measured_fslopes.is_distorted);
	
	std::cout << "solving homogeneous linear systems" << std::endl;
	cv::Mat_<real> A_h(measured_fslopes.count(), 3), A_v(measured_fslopes.count(), 3);
	std::ptrdiff_t row = 0;
	for(const auto& kv : measured_fslopes.slopes) {
		const std::string& feature_name = kv.first;
		const feature_point& fpoint = measured_fslopes.points.at(feature_name);
		const feature_slope& fslope = kv.second;

		A_h(row, 0) = -fslope.horizontal * intr.fx();
		A_h(row, 1) = intr.fy();
		A_h(row, 2) = intr.cy() - fpoint.position[1] - fslope.horizontal*(intr.cx() - fpoint.position[0]);
		
		A_v(row, 0) = intr.fx();
		A_v(row, 1) = -fslope.vertical * intr.fy();
		A_v(row, 2) = intr.cx() - fpoint.position[0] - fslope.vertical*(intr.cy() - fpoint.position[1]);
		
		row++;
	}
	vec3 r_1, r_2;
	cv::SVD::solveZ(A_h, r_1);
	cv::SVD::solveZ(A_v, r_2);
	
	std::cout << cv::norm(r_1) << std::endl;
	std::cout << cv::norm(r_2) << std::endl;
	
	std::cout << "forming rotation matrix" << std::endl;
	mat33 R;
	for(std::ptrdiff_t i = 0; i < 3; ++i) {
		R(i, 0) = r_1[i];
		R(i, 1) = r_2[i];
		R(i, 2) = std::sqrt(1.0 - sq(r_1[i]) - sq(r_2[i]));
	}

	std::cout << "saving rotation matrix" << std::endl;
	export_json_file(encode_mat(R), out_rotation_filename);
}
