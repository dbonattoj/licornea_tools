#include <string>
#include <map>
#include <cmath>
#include "../lib/utility/misc.h"
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
	std::map<std::string, vec2> feature_points;
	{
		json j_feature_points = import_json_file(feature_points_filename);
		for(auto it = j_feature_points.begin(); it != j_feature_points.end(); ++it) {
			const std::string& feature_name = it.key();
			const json& j_point = it.value();
			feature_points[feature_name] = vec2(j_point["x"], j_point["y"]);
		}
	}
	
	std::map<std::string, real> predicted_hslopes;
	std::map<std::string, real> predicted_vslopes;
	
	std::cout << "calculating predicted slopes" << std::endl;
	real fx = intr.fx(), fy = intr.fy(), cx = intr.cx(), cy = intr.cy();
	real r11 = R(0, 0), r21 = R(1, 0), r31 = R(2, 0);
	real r12 = R(0, 1), r22 = R(1, 1), r32 = R(2, 1);
	for(const auto& kv : feature_points) {
		const std::string& feature_name = kv.first;
		vec2 distorted_point = kv.second;
		vec2 undistorted_point = undistort_point(intr, distorted_point);
		real ix = undistorted_point[0], iy = undistorted_point[1];
		
		real hslope = (fy*r21 + cy*r31 - iy*r31) / (fx*r11 + cx*r31 - ix*r31);
		real vslope = (fx*r12 + cx*r32 - ix*r32) / (fy*r22 + cy*r32 - iy*r32);
		
		predicted_hslopes[feature_name] = hslope;
		predicted_vslopes[feature_name] = vslope;
	}
	

	std::cout << "saving predicted slopes" << std::endl;
	json j_feature_slopes = json::object();
	for(const auto& kv : feature_points) {
		const std::string& feature_name = kv.first;
		const vec2& distorted_point = kv.second;
		json j_slope = json::object();
		j_slope["ix"] = distorted_point[0];
		j_slope["iy"] = distorted_point[1];
		j_slope["horizontal"] = predicted_hslopes.at(feature_name);
		j_slope["vertical"] = predicted_vslopes.at(feature_name);
		j_feature_slopes[feature_name] = j_slope;
	}
	json j_slopes = json::object();
	j_slopes["slopes"] = j_feature_slopes;
	j_slopes["view"] = ""; ////
	export_json_file(j_slopes, out_predicted_slopes_filename);
}
