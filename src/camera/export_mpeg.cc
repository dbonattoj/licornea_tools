#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "../lib/camera.h"
#include "lib/camera_mpeg.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: export_mpeg in_cameras.json out_cameras_mpeg.txt [no_convert]\n";
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string in_cameras = argv[1];
	std::string out_cameras = argv[2];
	bool convert = true;
	if(argc > 3) {
		std::string no_convert = argv[3];
		if(no_convert == "no_convert") convert = false;
		else usage_fail();
	}
	
	auto cameras = read_cameras_file(in_cameras);
	
	std::ofstream output(out_cameras);
	for(const camera& cam : cameras) write_camera_mpeg(output, cam, convert);
}
