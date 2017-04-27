#include <iostream>
#include <string>
#include <map>
#include "lib/image_correspondence.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/eigen.h"


using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cameras_on_line dataset_parameters.json image_correspondences.json intrinsics.json out_cams.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cams_filename = argv[4];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading intrinsic matrix" << std::endl;
	Eigen_mat3 K = decode_mat<real, 3, 3>(import_json_file(intrinsics_filename));
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	
	real ox = -cx/fx, oy = -cy/fy;
	
	std::map<view_index, std::vector<Eigen_vec2>> views_centers;

	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	view_index reference_view_idx = cors.reference;
	for(const auto& kv : cors.features) {
		std::string feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		Eigen_vec2 ref_i = feature.points.at(reference_view_idx);
		real ref_sx = ref_i[0] / fx, ref_sy = ref_i[1] / fy;
		
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
		for(auto& kv : view_samples) {
			Eigen_vec2& s = kv.second;
			s[0] -= ref_sx;
			s[1] -= ref_sy;
		}
		
		// remove scaling
		for(auto& kv : view_samples) {
		}
		real d = 1000.0;
		d = feature.depth;
		
		for(const auto& kv : view_samples) {
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
		
		Eigen_vec2 mean(0.0, 0.0);
		for(Eigen_vec2 center : centers) mean += center;
		mean /= centers.size();
		
		views_final_center[idx] = centers[0];
		idx.x += 10000; views_final_center[idx] = centers[1];
		idx.x += 10000; views_final_center[idx] = centers[2];
		idx.x += 10000; views_final_center[idx] = centers[3];
	}
	
	
	
	std::vector<camera> cameras;
	for(const auto& kv : views_final_center) {
		view_index idx = kv.first;
		Eigen_vec2 t = kv.second;
		
		camera cam;
		cam.name = "cam";//datas.view(idx).camera_name();
		cam.intrinsic = K;
		cam.extrinsic <<
			1, 0, 0, t[0],
			0, 1, 0, t[1],
			0, 0, 1, 0,
			0, 0, 0, 1;
		cameras.push_back(cam);
	}
	
	
	write_cameras_file(out_cams_filename, cameras);
}

