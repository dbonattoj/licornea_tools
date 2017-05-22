#include <string>
#include <map>
#include <cmath>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_predict_optical_flow_slopes rotation.json intrinsics.json dataset_parameters.json measured_slopes.json out_predicted_slopes.json\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	
}
