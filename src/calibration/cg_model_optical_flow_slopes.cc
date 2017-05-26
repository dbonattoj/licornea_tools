#include <string>
#include <map>
#include <cmath>
#include "lib/cg/feature_slopes.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_predict_optical_flow_slopes feature_points.json intrinsics.json rotation.json out_predicted_slopes.json\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string feature_points_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string rotation_filename = argv[3];
	std::string out_predicted_slopes_filename = argv[4];
	
	std::cout << "loading intrinsics, rotation" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));
	mat33 R = decode_mat(import_json_file(rotation_filename));

	std::cout << "loading feature points" << std::endl;
	feature_points fpoints = decode_feature_points(import_json_file(feature_points_filename));	

	std::cout << "saving predicted slopes" << std::endl;
	feature_slopes fslopes(fpoints);
	for(const auto& kv : fslopes.points) {
		const std::string& feature_name = kv.first;
		const feature_point& fpoint = kv.second;
		feature_slope& fslope = fslopes.slopes[feature_name];
		fslope.horizontal = model_horizontal_slope(fpoint.undistorted_point, intr.K, R);
		fslope.vertical = model_vertical_slope(fpoint.undistorted_point, intr.K, R);
	}
	export_json_file(encode_feature_slopes(fslopes), out_predicted_slopes_filename);
}
