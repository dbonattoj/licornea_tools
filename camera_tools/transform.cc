#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "lib/eigen.h"
#include "lib/camera.h"

[[noreturn]] void usage_fail() {
	std::cout << "usage: camera_transform in_cameras.txt out_cameras.txt operation\n";
	std::cout << "operations: Rt2MPEG: standard extrinsic matrix to MPEG convention\n";
	std::cout << "            MPEG2Rt: MPEG convention to standard extrinsic matrix\n";
	std::cout << "            flip_t: flip sign of translation vectors\n";
	std::cout << "            scale old new: adapt intrinsic matrix for image scale from old to new\n";
	std::cout << "                           (old/new = pixel length of same segment in old and new image)\n";
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
	std::ofstream output(out_cameras.c_str());
	input.exceptions(std::ios_base::badbit);	
	
	camera cam;
	
	while(read_camera(input, cam)) {
		if(operation == "Rt2MPEG") {
			cam.translation = -(cam.rotation.inverse() * cam.translation);
			
		} else if(operation == "MPEG2Rt") {
			cam.translation = -(cam.rotation * cam.translation);	
					
		} else if(operation == "flip_t") {
			cam.translation = -cam.translation;			
			
		} else if(operation == "scale") {
			if(argc <= 5) usage_fail();
			int old_sz = std::atoi(argv[4]);
			int new_sz = std::atoi(argv[5]);
			float factor = (float)new_sz / old_sz;
			cam.intrinsic(0,0) *= factor;
			cam.intrinsic(1,1) *= factor;
			cam.intrinsic(0,2) *= factor;
			cam.intrinsic(1,2) *= factor;
			
		} else if(operation == "nop") {
			// no change
		} else {
			usage_fail();
		}
		
		write_camera(output, cam);
	}
	
	std::cout << "done" << std::endl;
}
