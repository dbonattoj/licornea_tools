#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "lib/cg/feature_points.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

const bool verbose = false;

int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"dataset_parameters.json cors.json intr.json R.json straight_depths.json x_step y_step out_cors.json out_cameras.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	json j_feature_straight_depths = json_arg();
	real x_step = real_arg();
	real y_step = real_arg();
	std::string out_cors_filename = out_filename_arg();
	std::string out_cameras_filename = out_filename_arg();
			
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion for cg_rectification_homographies");
	
	std::cout << "getting reference view points" << std::endl;
	view_index reference_idx = cors.reference;
	feature_points reference_fpoints = feature_points_for_view(cors, reference_idx);	
	
	std::cout << "calculating destination points and homography for each view" << std::endl;
	camera_array cameras;
	image_correspondences out_cors;
	out_cors.reference = reference_idx;	
	out_cors.dataset_group = "rectified";
	mat33 M = intr.K * R.t() * intr.K_inv;
	real total_reprojection_error = 0;
	int total_reprojection_error_samples = 0;
	for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
		view_index target_idx(x, y);

		std::vector<vec2> source_points, destination_points;
		dataset_view target_view = datas.view(target_idx);

		feature_points target_source_fpoints = undistorted_feature_points_for_view(cors, target_idx, intr);	

		// get source point, and compute destination point for each feature
		real x_translation = (target_idx.x - reference_idx.x) * x_step;
		real y_translation = (target_idx.y - reference_idx.y) * y_step;
		for(auto& kv : reference_fpoints.points) {
			const std::string& feature_name = kv.first;
			const vec2& reference_fpoint = kv.second;
	
			// get source point
			if(! target_source_fpoints.has(feature_name)) continue;
			vec2 source_point = target_source_fpoints.points.at(feature_name);
			
			// get straight depth
			if(! has(j_feature_straight_depths, feature_name)) continue;
			real straight_depth = j_feature_straight_depths[feature_name];

			// remove rotation
			vec3 rot_i_h(reference_fpoint[0], reference_fpoint[1], 1.0);
			vec3 no_i_h = M * rot_i_h;
			vec2 no_i(no_i_h[0] / no_i_h[2], no_i_h[1] / no_i_h[2]);
	
			// translate
			vec2 tr_i;
			tr_i[0] = no_i[0] + (x_translation * intr.fx() / straight_depth);
			tr_i[1] = no_i[1] + (y_translation * intr.fy() / straight_depth);

			source_points.push_back(source_point);
			destination_points.push_back(tr_i);
			
			// write output correspondence
			image_correspondence_feature& feature = out_cors.features[feature_name];
			feature.point_depths[target_idx] = straight_depth;
			feature.points[target_idx] = tr_i;
		}
		

		// compute camera position for rectified view
		camera cam;
		cam.name = target_view.camera_name();
		cam.intrinsic = intr.K;
		cam.rotation = mat33::eye();
		cam.translation = vec3(x_translation, y_translation, 0.0);
		cameras.push_back(cam);
	}
	std::cout << std::endl;
	
	std::cout << "saving destination image correspondences" << std::endl;
	export_json_file(encode_image_correspondences(out_cors), out_cors_filename);

	std::cout << "saving cameras" << std::endl;
	write_cameras_file(out_cameras_filename, cameras);
	
	std::cout << "done" << std::endl;
}
