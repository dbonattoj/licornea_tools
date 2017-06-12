#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "lib/image_correspondence.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cors.json out_cors.json ");
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	export_image_corresponcences(cors, out_cors_filename);
}

