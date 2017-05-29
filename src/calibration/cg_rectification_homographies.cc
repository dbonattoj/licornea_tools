#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "lib/cg/feature_points.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

const bool verbose = true;

int main(int argc, const char* argv[]) {
	auto args = get_args(argc, argv,
		"dataset_parameters.json cors.json intr.json R.json straight_depths.json x_step y_step out_homographies.json out_cameras.json [out_cors.json]");
	std::string datas_filename = args.in_filename_arg();
	std::string cors_filename = args.in_filename_arg();
	std::string intr_filename = args.in_filename_arg();
	std::string R_filename = args.in_filename_arg();
	std::string straight_depths_filename = args.in_filename_arg();
	real x_step = args.real_arg();
	real y_step = args.real_arg();
	std::string out_homographies_filename = args.out_filename_arg();
	std::string out_cameras_filename = args.out_filename_arg();
	std::string out_cors_filename;
	if(args.has_next_arg()) out_cors_filename = args.out_filename_arg();
		
	std::cout << "loading parameters" << std::endl;
	dataset datas(datas_filename);
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	intrinsics intr = decode_intrinsics(import_json_file(intr_filename));
	mat33 R = decode_mat(import_json_file(R_filename));
	json j_feature_straight_depths = import_json_file(straight_depths_filename);
	
	std::cout << "getting reference view points" << std::endl;
	view_index reference_idx = cors.reference;
	feature_points reference_fpoints = feature_points_for_view(cors, reference_idx, intr);	
	
	
	std::cout << "calculating destination points and homography for each view" << std::endl;
	std::map<view_index, mat33> homographies;
	camera_array cameras;
	image_correspondences out_cors;
	out_cors.reference = reference_idx;	
	mat33 M = intr.K * R * intr.K_inv;
	real total_reprojection_error = 0;
	int total_reprojection_error_samples = 0;
	for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
		view_index target_idx(x, y);

		std::vector<vec2> source_points, destination_points;
		dataset_view target_view = datas.view(target_idx);

		feature_points target_source_fpoints = feature_points_for_view(cors, target_idx, intr);	

		// get source point, and compute destination point for each feature
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
			tr_i[0] = no_i[0] + (x_translation * intr.fx() / straight_depth);
			tr_i[1] = no_i[1] + (y_translation * intr.fy() / straight_depth);

			source_points.push_back(source_point);
			destination_points.push_back(tr_i);
						
			image_correspondence_feature& feature = out_cors.features[feature_name];
			feature.depth = straight_depth;
			feature.points[target_idx] = tr_i; // TODO this is undistorted, og cors is distorted. make cors always undistorted
		}
		
		// compute homography for this view
		mat33 homography = cv::findHomography(source_points, destination_points, 0, 0);

		// compute reprojection error
		{
			std::vector<vec2> warped_source_points;
			cv::perspectiveTransform(source_points, warped_source_points, homography);
			real err = 0.0;
			for(std::ptrdiff_t i = 0; i < source_points.size(); ++i) {
				const vec2& warped = warped_source_points.at(i);
				const vec2& dest = destination_points.at(i);
				err += sq(warped[0] - dest[0]) + sq(warped[1] - dest[1]);
			}
			total_reprojection_error += err;
			total_reprojection_error_samples += source_points.size();
			
			if(verbose) {
				std::cout << "target view " << target_idx << "\n";
				std::cout << "feature count: " << source_points.size() << "\n";
				std::cout << "homography reprojection error: " << std::sqrt(err / source_points.size()) << "\n" << std::endl;
			} else {
				std::cout << '.' << std::flush;
			}
		}
		
		camera cam;
		cam.name = target_view.camera_name();
		cam.intrinsic = intr.K;
		cam.rotation = mat33::eye();
		cam.translation = vec3(x_translation, y_translation, 0.0);

		homographies[target_idx] = homography;
		cameras.push_back(cam);
	}
	std::cout << std::endl;
	
	total_reprojection_error = std::sqrt(total_reprojection_error / total_reprojection_error_samples);
	std::cout << "total reprojection error: " << total_reprojection_error << std::endl;
	
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
	
	if(! out_cors_filename.empty()) {
		std::cout << "saving destination image correspondences" << std::endl;
		export_image_correspondences_file(out_cors_filename, out_cors);
	}
	
	std::cout << "done" << std::endl;
}
