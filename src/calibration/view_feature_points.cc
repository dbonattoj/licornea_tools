#include <string>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: view_feature_points image_correspondences.json out_feature_points.json [x_index] [y_index]\n" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string cors_filename = argv[1];
	std::string out_feature_points_filename = argv[2];
	int x_index = -1, y_index = -1;
	if(argc > 3) x_index = std::atoi(argv[3]);
	if(argc > 4) y_index = std::atoi(argv[4]);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);

	std::cout << "loading feature points" << std::endl;
	view_index idx;
	if(x_index == -1) idx = cors.reference;
	else if(y_index == -1) idx = view_index(x_index);
	else idx = view_index(x_index, y_index);

	json j_feature_points = json::object();
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		const vec2& point = feature.points.at(idx);
		json j_point = json::object();
		j_point["x"] = point[0];
		j_point["y"] = point[1];
		j_feature_points[feature_name] = j_point;
	}
	
	std::cout << "saving feature points" << std::endl;
	export_json_file(j_feature_points, out_feature_points_filename);
}
