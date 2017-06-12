int main(){}
/*
#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "lib/feature_points.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>

using namespace tlz;

const bool verbose = false;

mat33 unrotate_out_R(const mat33& R, const std::string& directions) {
	if(directions == "horizontal") {
		vec3 n(1, 0, 0); // normal to plane x=0
		vec3 d = R * vec3(0, 0, 1);
		vec3 yd = d - (n * d.dot(n));
		yd = yd / std::sqrt(sq(yd[1]) + sq(yd[2]));	
		mat33 Rx(
			1.0, 0.0, 0.0,
			0.0, yd[2], -yd[1],
			0.0, yd[1], yd[2]
		);
		return Rx.t();
	}
	else if(directions == "vertical") {
		vec3 n(0, 1, 0); // normal to plane y=0
		vec3 d = R * vec3(0, 0, 1);
		vec3 xd = d - (n * d.dot(n));
		xd = xd / std::sqrt(sq(xd[0]) + sq(xd[2]));
		mat33 Ry(
			xd[2], 0.0, xd[0],
			0.0, 1.0, 0.0,
			-xd[0], 0.0, xd[2]
		);
		return Ry.t();
		
	} else {
		Assert(directions == "both");
		return mat33::eye();
	}
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"dataset_parameters.json cors.json intr.json R.json straight_depths.json out_cors.json out_cameras.json leave/unrotate/align [horizontal/vertical/both]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	json j_feature_straight_depths = json_arg();
	std::string out_cors_filename = out_filename_arg();
	std::string out_cameras_filename = out_filename_arg();
	std::string mode = enum_arg({ "leave", "unrotate", "align" });
	std::string directions = enum_opt_arg({ "horizontal", "vertical", "both" }, "both");
	
	
	
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	std::cout << "getting reference view points" << std::endl;
	view_index reference_idx = cors.reference;
	feature_points reference_fpoints = feature_points_for_view(cors, reference_idx);
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	std::cout << "collecting reference and target view feature data" << std::endl;
	struct view_feature {
		std::string name;
		real straight_depth;
		vec2 reference_point;
		vec2 straight_reference_point;
		vec2 target_source_point;
		vec2 straight_target_source_point;
	};
	using view_features = std::vector<view_feature>;
	std::map<view_index, view_features> features;

	for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
		view_index target_idx(x, y);
		feature_points target_source_fpoints = feature_points_for_view(cors, target_idx);	
		for(auto& kv : reference_fpoints.points) {
			const std::string& feature_name = kv.first;
			if(! target_source_fpoints.has(feature_name)) continue;
			if(! has(j_feature_straight_depths, feature_name)) continue;
			
			view_feature feature;
			feature.name = feature_name;
			feature.reference_point = kv.second;
			feature.target_source_point = target_source_fpoints.points.at(feature_name);
			feature.straight_depth = j_feature_straight_depths[feature_name];
			feature.straight_reference_point = mul_h(unrotate, feature.reference_point);
			feature.straight_target_source_point = mul_h(unrotate, feature.target_source_point);
			features[target_idx].push_back(feature);
		}
	}
	
	camera_array cameras;
	image_correspondences out_cors;
	out_cors.reference = reference_idx;	
	out_cors.dataset_group = "warped"; /////

	std::cout << "computing camera positions and destination correspondences" << std::endl;
	if(mode == "leave" || mode == "unrotate") {
		mat33 out_R, rerotate;
		if(mode == "leave") {
			out_R = R;
			rerotate = intr.K * R * intr.K_inv;
		} else if(mode == "unrotate") {	
			out_R = unrotate_out_R(R, directions);
			rerotate = intr.K * out_R * intr.K_inv;	
		}
		
		for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
			view_index target_idx(x, y);
	
			vec2 camera_position(0.0, 0.0);
			for(const view_feature& feature : features.at(target_idx)) {
				vec2 feature_camera_position;
				feature_camera_position[0] = (feature.straight_target_source_point[0] - feature.straight_reference_point[0]) * feature.straight_depth / intr.fx();
				feature_camera_position[1] = (feature.straight_target_source_point[1] - feature.straight_reference_point[1]) * feature.straight_depth / intr.fy();
				camera_position += feature_camera_position;
			}
			camera_position = camera_position / (real)features.at(target_idx).size();
			for(const view_feature& feature : features.at(target_idx)) {
				vec2 straight_target_destination_point;
				straight_target_destination_point[0] = feature.straight_reference_point[0] + (camera_position[0] * intr.fx()) / feature.straight_depth;
				straight_target_destination_point[1] = feature.straight_reference_point[1] + (camera_position[1] * intr.fy()) / feature.straight_depth;
				
				vec2 target_destination_point = mul_h(rerotate, straight_target_destination_point);
				
				image_correspondence_feature& cors_feature = out_cors.features[feature.name];
				cors_feature.points[target_idx] = target_destination_point;
				cors_feature.point_depths[target_idx] = feature.straight_depth;
			}
			
			camera cam;
			cam.name = datas.view(target_idx).camera_name();
			cam.intrinsic = intr.K;
			cam.rotation = R;
			cam.translation = R * vec3(camera_position[0], camera_position[1], 0.0);
			cameras.push_back(cam);
		}
	
	} else if(mode == "align") {
		// ...
	}
	
	std::cout << "saving destination image correspondences" << std::endl;
	export_json_file(encode_image_correspondences(out_cors), out_cors_filename);

	std::cout << "saving cameras" << std::endl;
	write_cameras_file(out_cameras_filename, cameras);
	
	std::cout << "done" << std::endl;
}
*/
