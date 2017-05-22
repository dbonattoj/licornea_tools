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


int main(int argc, const char* argv[]) {
}
