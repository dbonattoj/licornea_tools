#include <string>
#include <map>
#include <cmath>
#include <random>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_simulation rotation.json intrinsics.json out_image_correspondences.json [out_cams.json] [out_visualisation.ply] [features_count=100] [num_x=100] [num_y=20] [step_x=1.0] [step_y=1.0]\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string rotation_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string out_cors_filename = argv[3];
	std::string out_cameras_filename;
	std::string out_visualization_filename;
	int features_count = 100;
	int num_x = 100;
	int num_y = 20;
	real step_x = 1;
	real step_y = 1;
	if(argc > 4) out_cameras_filename = argv[4];
	if(argc > 5) out_visualization_filename = argv[4];
	if(argc > 6) features_count = std::atoi(argv[6]);
	if(argc > 7) num_x = std::atoi(argv[7]);
	if(argc > 8) num_y = std::atoi(argv[8]);
	if(argc > 9) step_x = std::atof(argv[9]);
	if(argc > 10) step_y = std::atof(argv[10]);
	
	mat33 R = decode_mat(import_json_file(rotation_filename));
	mat33 K = decode_mat(import_json_file(intrinsics_filename));
	
	std::cout << "generating " << features_count << " random 3D features" << std::endl;
	std::vector<vec3> features;
	{
		std::uniform_real_distribution<real> x_dist(-step_x*num_x/1.2, +step_x*num_x/1.2);
		std::uniform_real_distribution<real> y_dist(-step_y*num_y/1.2, +step_y*num_y/1.2);
		std::uniform_real_distribution<real> z_dist(500.0, 2000.0);
		std::mt19937 gen;

		for(int i = 0; i < features_count; ++i) {
			vec3 feature(x_dist(gen), y_dist(gen), z_dist(gen));
			features.push_back(feature);
		}
	}
	
	std::cout << "projecting features for each view" << std::endl;
	image_correspondences cors;
	std::map<view_index, vec3> view_camera_centers;
	for(int y = 0; y < num_y; ++y) for(int x = 0; x < num_x; ++x) {
		view_index idx(x, y);
		
		real px = step_x*(x - num_x/2);
		real py = step_y*(y - num_y/2);
		vec3 p(px, py, 0.0);
		view_camera_centers[idx] = p;
		
		for(int feature = 0; feature < features_count; ++feature) {
			const vec3& w = features[feature];
			vec3 v = R*(w + p);
			vec3 i_ = K*v;
			vec2 i(i_[0]/i_[2], i_[1]/i_[2]);
						
			std::string feature_name = "feat" + std::to_string(feature);
			cors.features[feature_name].points[idx] = i;
		}
	}
	
	std::cout << "saving correspondences" << std::endl;
	export_image_correspondences_file(out_cors_filename, cors);
	
	if(! out_cameras_filename.empty()) {
		std::cout << "saving cameras" << std::endl;
		camera_array cams;
		for(const auto& kv : view_camera_centers) {
			view_index idx = kv.first;
			const vec3& p = kv.second;
			vec3 t = R * p;
			
			camera cam;
			cam.name = encode_view_index(idx);
			cam.intrinsic = K;
			cam.rotation = R;
			cam.translation = t;
			cams.push_back(cam);
		}
		write_cameras_file(out_cameras_filename, cams);
	}
	
	
	if(! out_visualization_filename.empty()) {
		// TODO
	}
	
	std::cout << "done" << std::endl;
}
