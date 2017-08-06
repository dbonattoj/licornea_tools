#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../lib/args.h"
#include "../lib/assert.h"
#include "../lib/camera.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cameras1.json in_cameras2.json out_cameras.json [common_camera]");
	camera_array cameras1 = cameras_arg();
	camera_array cameras2 = cameras_arg();
	std::string out_cameras_filename = out_filename_arg();
	std::string common_camera_name = string_opt_arg();
		
	auto cameras1_map = cameras_map(cameras1);
	auto cameras2_map = cameras_map(cameras2);
	
	camera_array out_cameras = cameras1;
	
	if(common_camera_name.empty()) {
		for(const camera& cam : cameras2) {
			const std::string& name = cam.name;
			if(cameras1_map.find(name) != cameras1_map.end()) continue;
			out_cameras.push_back(cam);
		}
		
	} else {
		if(cameras1_map.find(common_camera_name) == cameras1_map.end())
			throw std::runtime_error("common_camera not in in_cameras1.json");
		if(cameras2_map.find(common_camera_name) == cameras2_map.end())
			throw std::runtime_error("common_camera not in in_cameras2.json");
		
		const camera& cam1 = cameras1_map.at(common_camera_name);
		const camera& cam2 = cameras2_map.at(common_camera_name);
		mat44 transformation = cam1.extrinsic() * cam2.extrinsic_inv();

		for(const camera& cam : cameras2) {
			const std::string& name = cam.name;
			if(cameras1_map.find(name) != cameras1_map.end()) continue;
			camera transformed_cam = cam;
			mat44 ex = transformed_cam.extrinsic();
			transformed_cam.set_extrinsic(transformation * ex);
			out_cameras.push_back(transformed_cam);
		}

	}
	
	export_cameras_file(out_cameras, out_cameras_filename);
}


