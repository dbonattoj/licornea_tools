#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <format.h>

#include "../lib/eigen.h"
#include "../lib/camera.h"
#include "../lib/camera_mpeg.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: transform in_cameras.json out_cameras.json/none/replace operation\n";
	std::cout << "operations: Rt2MPEG: standard extrinsic matrix to MPEG convention\n";
	std::cout << "            MPEG2Rt: MPEG convention to standard extrinsic matrix\n";
	std::cout << "            flip_t: flip sign of translation vectors\n";
	std::cout << "            scale old new: adapt intrinsic matrix for image scale from old to new\n";
	std::cout << "                           (old/new = pixel length of same segment in old and new image)\n";
	std::cout << "            rename cam_{} [offset=0] [factor=1]: rename according to format, with new_index = offset + factor*old_index\n";
	std::cout << "            nop: No change, just reformat the cameras file\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string in_cameras = argv[1];
	std::string out_cameras = argv[2];
	std::string operation = argv[3];
		
	std::ifstream input(in_cameras.c_str());
	input.exceptions(std::ios_base::badbit);	
	auto cameras = read_cameras_file(in_cameras);
	input.close();

	int index = 0;
	for(camera& cam : cameras) {
		if(operation == "Rt2MPEG") {
			cam.translation() = -(cam.rotation().inverse() * cam.translation());
			
		} else if(operation == "MPEG2Rt") {
			cam.translation() = -(cam.rotation() * cam.translation());	
					
		} else if(operation == "flip_t") {
			cam.translation() = -cam.translation();			
			
		} else if(operation == "scale") {
			if(argc <= 5) usage_fail();
			int old_sz = std::atoi(argv[4]);
			int new_sz = std::atoi(argv[5]);
			float factor = (float)new_sz / old_sz;
			cam.intrinsic(0,0) *= factor;
			cam.intrinsic(1,1) *= factor;
			cam.intrinsic(0,2) *= factor;
			cam.intrinsic(1,2) *= factor;
			
		} else if(operation == "rename") {
			if(argc <= 4) usage_fail();
			std::string format = argv[4];
			int offset = 0;
			int factor = 1;
			if(argc > 5) offset = std::atoi(argv[5]);
			if(argc > 6) factor = std::atoi(argv[6]);
			std::string new_name = fmt::format(format, factor*index + offset);
			std::cout << cam.name << " --> " << new_name << std::endl;
			cam.name = new_name;
			
		} else if(operation == "nop") {
			// no change
		} else {
			usage_fail();
		}
		++index;
	}
	
	if(out_cameras == "none") {
		std::cout << "not writing to output" << std::endl;
	} else {
		if(out_cameras == "replace") out_cameras = in_cameras;
		std::cout << "writing to " << out_cameras << std::endl;
		std::ofstream output(out_cameras.c_str());
		write_cameras_file(out_cameras, cameras);
		output.close();
	}
	
	std::cout << "done" << std::endl;
}
