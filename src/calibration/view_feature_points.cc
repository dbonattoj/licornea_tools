#include <string>
#include "lib/image_correspondence.h"
#include "lib/cg/feature_points.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: view_feature_points image_correspondences.json intrinsics.json out_feature_points.json [x_index] [y_index]\n" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string cors_filename = argv[1];
	std::string intr_filename = argv[2];
	std::string out_feature_points_filename = argv[3];
	int x_index = -1, y_index = -1;
	if(argc > 4) x_index = std::atoi(argv[4]);
	if(argc > 5) y_index = std::atoi(argv[5]);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);

	std::cout << "loading intrinsics" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intr_filename));

	std::cout << "saving feature points" << std::endl;
	view_index idx;
	if(x_index == -1) idx = cors.reference;
	else if(y_index == -1) idx = view_index(x_index);
	else idx = view_index(x_index, y_index);
	feature_points fpoints = feature_points_for_view(cors, idx, intr);
	export_json_file(encode_feature_points(fpoints), out_feature_points_filename);

	std::cout << "done" << std::endl;
}
