#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "lib/cg/feature_points.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

int main(int argc, const char* argv[]) {
	auto args = get_args(argc, argv,
		"dataset_parameters.json cors.json intr.json R.json straight_depths.json x_step y_step out_homographies.json out_cameras.json");
	std::string datas_filename = args.in_filename_arg();
	std::string cors_filename = args.in_filename_arg();
	std::string intr_filename = args.in_filename_arg();
	std::string R_filename = args.in_filename_arg();
	std::string straight_depths_filename = args.in_filename_arg();
	real x_step = args.real_arg();
	real y_step = args.real_arg();
	std::string out_homographies_filename = args.out_filename_arg();
	std::string out_cameras_filename = args.out_filename_arg();
		
	std::cout << "loading parameters" << std::endl;
	dataset datas(datas_filename);
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	intrinsics intr = decode_intrinsics(import_json_file(intr_filename));
	mat33 R = decode_mat(import_json_file(R_filename));
	json j_feature_straight_depths = import_json_file(straight_depths_filename);
	
	std::cout << "getting reference view points" << std::endl;
	view_index reference_idx = cors.reference;
	feature_points reference_fpoints = feature_points_for_view(cors, reference_idx, intr);	

	struct view_result {
		mat33 homography;
		camera cam;
	};
	auto process_target_view = [&](view_index target_idx) -> view_result {
		std::vector<vec2> source_points, destination_points;
		dataset_view target_view = datas.view(target_idx);

		feature_points target_source_fpoints = feature_points_for_view(cors, target_idx, intr);	

		mat33 M = intr.K * R * intr.K_inv;
		real x_translation = (target_idx.x - reference_idx.x) * x_step;
		real y_translation = (target_idx.y - reference_idx.y) * y_step;
		for(auto& kv : reference_fpoints.points) {
			const std::string& feature_name = kv.first;
			const feature_point& reference_fpoint = kv.second;
	
			// get source point
			if(! target_source_fpoints.has(feature_name)) continue;
			vec2 source_point = target_source_fpoints.points.at(feature_name).undistorted_point;
			
			// get straight depth
			if(! has(j_feature_straight_depths, feature_name)) continue;
			real straight_depth = j_feature_straight_depths[feature_name];

			// remove rotation
			vec3 rot_i_h(reference_fpoint.undistorted_point[0], reference_fpoint.undistorted_point[1], 1.0);
			vec3 no_i_h = M * rot_i_h;
			vec2 no_i(no_i_h[0] / no_i_h[2], no_i_h[1] / no_i_h[2]);
	
			// translate
			vec2 tr_i;
			tr_i[0] = no_i[0] - (x_translation * intr.fx() / straight_depth);
			tr_i[1] = no_i[1] - (y_translation * intr.fy() / straight_depth);

			source_points.push_back(source_point);
			destination_points.push_back(tr_i);
		}
		
		mat33 homography = cv::findHomography(source_points, destination_points, 0, 0);
		
		camera cam;
		cam.name = target_view.camera_name();
		cam.intrinsic = intr.K;
		cam.rotation = mat33::eye();
		cam.translation = vec3(x_translation, y_translation, 0.0);
		
		return { homography, cam };
	};
	
	
	std::cout << "calculating destination points and homography for each view" << std::endl;
	std::map<view_index, mat33> homographies;
	camera_array cameras;
	for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
		std::cout << '.' << std::flush;
		view_index idx(x, y);
		view_result result = process_target_view(idx);
		homographies[idx] = result.homography;
		cameras.push_back(result.cam);
	}
	std::cout << std::endl;
	
	
	std::cout << "saving homographies" << std::endl;
	{
		json j_homographies = json::object();
		for(const auto& kv : homographies) {
			const view_index& idx = kv.first;
			const mat33& homography = kv.second;
			j_homographies[encode_view_index(idx)] = encode_mat(homography);
		}
		export_json_file(j_homographies, out_homographies_filename);
	}
	
	
	std::cout << "saving cameras" << std::endl;
	write_cameras_file(out_cameras_filename, cameras);
	
	std::cout << "done" << std::endl;
}
