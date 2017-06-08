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
	get_args(argc, argv, "in_cameras_mpeg.txt out_cameras.json [no_convert]");
	std::string in_cameras = in_filename_arg();
	std::string out_cameras = out_filename_arg();
	bool convert = ! bool_opt_arg("no_convert", false);
	
	std::vector<camera> cameras;
	{
		std::ifstream input(in_cameras);
		camera cam;
		while(read_camera_mpeg(input, cam, convert)) cameras.push_back(cam);
	}
	
	write_cameras_file(out_cameras, cameras);
}
