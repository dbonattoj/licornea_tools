#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../lib/args.h"
#include "../lib/assert.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "parameters.json in_cameras.json out_cameras.json");
	dataset datas = dataset_arg();
	camera_array cams = cameras_arg();
	std::string out_cams_filename = out_filename_arg();
		
	auto cams_map = cameras_map(cams);
	camera_array out_cams;

	for(const view_index& idx : datas.indices()) {
		const std::string& camera_name = datas.view(idx).camera_name();
		auto cam_it = cams_map.find(camera_name);
		if(cam_it != cams_map.end()) out_cams.push_back(cam_it->second);
	}
	
	export_cameras_file(out_cams, out_cams_filename);
}


