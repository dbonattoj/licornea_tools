#include <string>
#include <map>
#include <cmath>
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_estimate_rotation slopes.json intrinsics.json out_rotation.json\n";
	std::cout << std::endl;
	std::exit(1);
}

struct flow_line {
	vec2 reference_view_point;
	real slope;
};

std::pair<real, real> estimate_pitch_roll_from_vertical_flow(const mat33& K, const image_correspondences& cors) {
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	view_index reference_idx = cors.reference;
	
	std::cout << "determining vertical flow lines" << std::endl;
	std::map<std::string, flow_line> feature_flow_lines;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		std::vector<vec2> points;
		for(const auto& kv2 : feature.points) points.push_back(kv.second);
		
		vec4 line_parameters;
		cv::fitLine(points, line_parameters, CV_DIST_L2, 0.0, 0.01, 0.01);
		
		feature_flow_lines[feature_name] = flow_line {
			feature.points.at(reference_idx),
			line_parameters[0] / line_parameters[1]
		};
	}
	
	real predicted = [&](const std::string& feature_name, real pitch, real roll) {
		vec2 i = feature_flow_lines.at(feature_name).reference_view_point;
		real ix = i[0], iy = i[1];
		real tanroll = std::tan(roll), sinpitch = std::sin(pitch), cospitch = std::cos(pitch);
		return ((ix - cx)*sinpitch + fx*tanroll) / (fx*cospitch + (iy - cy)*sinpitch);
	};
	real error = [&](real pitch, real roll) {
		real error = 0.0;
		for(const auto& kv : feature_flow_lines) {
			const std::string& feature_name = kv.first;
			const flow_line& line = kv.second;
			real predicted = predicted(feature_name, pitch, roll);
			real observed = line.slope;
			error += std::abs(observed - predicted);
		}
		return error;
	};
	
	
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string cors_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string out_rotation_filename = argv[3];
	
	std::cout << "loading intrinsic matrix" << std::endl;
	const json& j_intrinsics = import_json_file(intrinsics_filename);
	mat33 K = decode_mat(j_intrinsics["K"]);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);

	std::cout << "estimating pitch from vertical flow" << std::endl;
	estimate_pitch_roll_from_vertical_flow(datas, K, cors);
	
	
	std::cout << "estimating yaw from horizontal flow" << std::endl;
}
