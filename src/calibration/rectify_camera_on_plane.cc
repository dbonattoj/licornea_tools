int main() {}
#if 0
#include <string>
#include <map>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/eigen.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: rectify_camera_on_plane dataset_parameters.json image_correspondences.json intrinsics.json out_cams.json out_homographies.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cams_filename = argv[4];
	std::string out_homographies_filename = argv[5];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading intrinsic matrix" << std::endl;

	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	view_index reference_idx = cors.reference;

	real x_step = 1.0, y_step = 1.0;
	
	
	std::cout << "defining rectified feature points and calculating homographies" << std::endl;


	std::cout << "done" << std::endl;
}
#endif
