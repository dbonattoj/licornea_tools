#include <string>
#include <map>
#include <cmath>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_predict_optical_flow_slopes dataset_parameters.json intrinsics.json rotation.json measured_slopes.json out_predicted_slopes.json\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string dataset_parameters_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string rotation_filename = argv[3];
	std::string measured_slopes_filename = argv[4];
	std::string predicted_slopes_filename = argv[5];
	
	std::cout << "loading data set, intrinsics, rotation" << std::endl;
	dataset datas(dataset_parameters_filename);
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));
	mat33 rotation = decode_mat(import_json_file(rotation_filename));

	std::cout << "loading points of measured slope" << std::endl;
	
}
