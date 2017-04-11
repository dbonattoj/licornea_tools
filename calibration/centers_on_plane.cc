#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "lib/img2img_correspondence.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: centers_on_plane in_img2img_cors.json intrinsics.json cameras.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {

}

