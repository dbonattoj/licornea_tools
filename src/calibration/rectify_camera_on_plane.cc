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
	std::cout << "usage: rectify_camera_on_plane dataset_parameters.json image_correspondences.json intrinsics.json out_cams.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cams_filename = argv[4];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading intrinsic matrix" << std::endl;
	const json& j_intrinsics = import_json_file(intrinsics_filename);
	cv::Mat3d K = decode_mat_cv(j_intrinsics["K"]);
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);

}
