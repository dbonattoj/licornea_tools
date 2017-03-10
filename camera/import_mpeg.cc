#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "../lib/camera.h"
#include "../lib/camera_mpeg.h"

[[noreturn]] void usage_fail() {
	std::cout << "usage: import_mpeg in_cameras_mpeg.txt out_cameras.json [no_convert]\n";
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
	
	std::vector<camera> cameras;
	{
		std::ifstream input(in_cameras);
		camera cam;
		while(read_camera_mpeg(input, cam, convert)) cameras.push_back(cam);
	}
	
	write_cameras_file(out_cameras, cameras);
}
