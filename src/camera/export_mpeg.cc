#include "../lib/args.h"
#include "../lib/camera.h"
#include "lib/camera_mpeg.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iterator>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cameras.json out_cameras_mpeg.txt [no_convert]");
	camera_array in_cameras = cameras_arg();
	std::string out_cameras = out_filename_arg();
	bool convert = ! bool_opt_arg("no_convert", false);
		
	std::ofstream output(out_cameras);
	for(const camera& cam : in_cameras) write_camera_mpeg(output, cam, convert);
}
