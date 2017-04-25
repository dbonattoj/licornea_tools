#include <iostream>
#include <string>
#include <map>
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/image_correspondence.h"
#include "../lib/eigen.h"


using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: centers_on_plane dataset_parameters.json image_correspondences.json intrinsics.json out_cameras.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string image_correspondences_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cameras_filename = argv[4];
	
	std::cout << "loading intrinsic matrix" << std::endl;
	Eigen_mat3 K = decode_mat<real, 3, 3>(import_json_file(intrinsics_filename));
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	
	real ox = -cx/fx, oy = -cy/fy;
	
	std::map<view_index, std::vector<Eigen_vec2>> views_centers;

	std::cout << "loading correspondences" << std::endl;
	json j_features = import_json_file(image_correspondences_filename);
	for(json::iterator it = j_features.begin(); it != j_features.end(); ++it) {
		std::string feature_name = it.key();
		
		std::cout << feature_name << std::endl;
		
		image_correspondence_feature feature = decode_image_correspondence_feature(it.value());

		// load samples for this feature
		std::map<view_index, Eigen_vec2> view_samples;
		for(const auto& kv : feature.points) {
			view_index idx = kv.first;
			Eigen_vec2 i = kv.second;
			real ix = i[0], iy = i[1];

			real sx = ix / fx, sy = iy / fy;
			view_samples[idx] = Eigen_vec2(sx, sy);
		}
		
		// remove translation
		Eigen_vec2 dt(0, 0);
		for(const auto& kv : view_samples) dt += kv.second;
		dt /= view_samples.size();
		
		for(auto& kv : view_samples) {
			Eigen_vec2& s = kv.second;
			s -= dt;
		}
		
		// remove scaling
		real d = 1.0;
		
		for(const auto& kv : feature.points) {
			view_index idx = kv.first;
			Eigen_vec2 s = kv.second;
			real sx = s[0], sy = s[1];
			real tx = sx * d + ox, ty = sy * d + oy;
			views_centers[idx].emplace_back(tx, ty);
		}
	
	}


	std::map<view_index, Eigen_vec2> views_final_center;
	for(const auto& kv : views_centers) {
		view_index idx = kv.first;
		const auto& centers = kv.second;
		views_final_center[idx] = centers.front();
	}
	
	
	
	std::vector<camera> cameras;
	for(const auto& kv : views_final_center) {
		//view_index idx = kv.first;
		Eigen_vec2 t = kv.second;
		
		camera cam;
		cam.name = "cam";
		cam.intrinsic = K;
		cam.extrinsic <<
			1, 0, 0, t[0],
			0, 1, 0, t[1],
			0, 0, 1, 0,
			0, 0, 0, 1;
		cameras.push_back(cam);
	}
	
	
	write_cameras_file(out_cameras_filename, cameras);
}

