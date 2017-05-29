#include <string>
#include "lib/image_correspondence.h"
#include "lib/cg/feature_points.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: view_feature_points image_correspondences.json out_feature_points.json [x_index] [y_index]\n" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	auto args = get_args(argc, argv,
		"image_correspondences.json out_feature_points.json [x_index] [y_index]");
	std::string cors_filename = args.in_filename_arg();
	std::string out_feature_points_filename = args.out_filename_arg();
	int x_index = args.get_int_opt_arg(-1);
	int y_index = args.get_int_opt_arg(-1);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);

	std::cout << "saving feature points" << std::endl;
	view_index idx;
	if(x_index == -1) idx = cors.reference;
	else if(y_index == -1) idx = view_index(x_index);
	else idx = view_index(x_index, y_index);
	feature_points fpoints = feature_points_for_view(cors, idx, intr);
	export_json_file(encode_feature_points(fpoints), out_feature_points_filename);

	std::cout << "done" << std::endl;
}
