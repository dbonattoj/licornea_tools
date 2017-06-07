int main() {}
/*
#include <string>
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image_correspondences.json out_feature_points.json [x_index] [y_index]");
	image_correspondences cors = image_correspondences_arg();
	std::string out_feature_points_filename = out_filename_arg();
	int x_index = int_opt_arg(-1);
	int y_index = int_opt_arg(-1);
	
	std::cout << "saving feature points" << std::endl;
	view_index idx;
	if(x_index == -1) idx = cors.reference;
	else if(y_index == -1) idx = view_index(x_index);
	else idx = view_index(x_index, y_index);
	feature_points fpoints = feature_points_for_view(cors, idx);
	export_json_file(encode_feature_points(fpoints), out_feature_points_filename);

	std::cout << "done" << std::endl;
}
*/
