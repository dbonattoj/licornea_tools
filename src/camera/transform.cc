#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <format.h>
#include <regex>
#include "../lib/camera.h"
#include "../lib/border.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: transform in_cameras.json out_cameras.json/none/replace operation\n"
		<< "            MPEG2Rt: MPEG convention to standard extrinsic matrix\n"
		<< "            flip_t: flip sign of translation vectors\n"
		<< "            scale old new: adapt intrinsic matrix for image scale from old to new\n"
		<< "                           (old/new = pixel length of same segment in old and new image)\n"
		<< "            border [old] new: adapt intrinsic matrix for image border change from old to new\n"
		<< "                              (old/new = border.json file. if no old, assumed old border is zero)\n"
		<< "            rename cam_{} [offset=0] [factor=1]: rename according to format, with arg = offset + factor*index\n"
		<< "            head n: Only n first cameras\n"
		<< "            tail n: Only n last cameras\n"
		<< "            nop: No change, just rewrite the cameras file\n"
		<< std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string in_cameras_filename = argv[1];
	std::string out_cameras_filename = argv[2];
	std::string operation = argv[3];
	
	camera_array in_cameras = decode_cameras(import_json_file(in_cameras_filename));
	camera_array out_cameras;

	int index = 0;
	bool skip = false;
	
	for(camera& in_cam : in_cameras) {
		camera cam = in_cam;
		
		if(operation == "Rt2MPEG") {
			cam.translation = -(cam.rotation.t() * cam.translation);
						
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
		
		} else if(operation == "border") {
			border old_border, new_border;
			if(argc == 4) {
				std::string new_border_filename = argv[4];
				new_border = decode_border(import_json_file(new_border_filename));
			} else if(argc == 5) {
				std::string old_border_filename = argv[4];
				old_border = decode_border(import_json_file(old_border_filename));
				std::string new_border_filename = argv[5];
				new_border = decode_border(import_json_file(new_border_filename));
			} else usage_fail();
			cam.intrinsic(0,2) += new_border.left - old_border.left;
			cam.intrinsic(1,2) += new_border.top - old_border.top;
			
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
			
		} else if(operation == "head") {
			if(argc <= 4) usage_fail();
			int n = std::atoi(argv[4]);
			skip = (index >= n);
			
		} else if(operation == "tail") {
			if(argc <= 4) usage_fail();
			int n = std::atoi(argv[4]);
			skip = (index < in_cameras.size() - n);
		
		} else if(operation == "nop") {
			// no change
			
		} else {
			usage_fail();
		}
		++index;
		
		if(! skip) out_cameras.push_back(cam);
	}
		
	if(out_cameras_filename == "none") {
		std::cout << "not writing to output" << std::endl;
	} else {
		if(out_cameras_filename == "replace") out_cameras_filename = in_cameras_filename;
		std::cout << "writing to " << out_cameras_filename << std::endl;
		std::ofstream output(out_cameras_filename.c_str());
		export_cameras_file(out_cameras, out_cameras_filename);
		output.close();
	}
	
	std::cout << "done" << std::endl;
}
