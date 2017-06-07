#include <string>
#include <map>
#include <cmath>
#include "lib/cg/feature_slopes.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "feature_points.json intrinsics.json rotation.json out_predicted_slopes.json");
	feature_points fpoints = feature_points_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	std::string out_predicted_slopes_filename = out_filename_arg();

	fpoints = undistort(fpoints, intr);

	std::cout << "saving predicted slopes" << std::endl;
	feature_slopes fslopes(fpoints);
	for(const auto& kv : fslopes.points) {
		const std::string& feature_name = kv.first;
		const feature_point& fpoint = kv.second;
		feature_slope& fslope = fslopes.slopes[feature_name];
		fslope.horizontal = model_horizontal_slope(fpoint.position, intr.K, R);
		fslope.vertical = model_vertical_slope(fpoint.position, intr.K, R);
	}
	export_json_file(encode_feature_slopes(fslopes), out_predicted_slopes_filename);
}
