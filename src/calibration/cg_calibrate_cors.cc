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
#include "lib/image_correspondence.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <set>

using namespace tlz;

const bool verbose = false;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json intr.json R.json straight_depths.json out_cameras.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	json j_feature_straight_depths = json_arg();
	std::string out_cameras_filename = out_filename_arg();
	
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	auto all_vws = get_all_views(cors);
	auto ref_vws = get_reference_views(cors);
	
	std::cout << "estimating target camera positions, from each reference" << std::endl;
	using reference_view_index = view_index;
	using target_view_index = view_index;
	std::map<reference_view_index, std::vector<std::pair<target_view_index, vec2>>> reference_target_camera_positions;
	std::map<target_view_index, std::vector<std::pair<reference_view_index, vec2>>> target_reference_camera_positions;
		
	for(const view_index& ref_idx : ref_vws) {
		image_correspondences ref_cors = image_correspondences_with_reference(cors, ref_idx);
		feature_points ref_fpoints = feature_points_for_view(ref_cors, ref_idx, false);

		std::map<view_index, vec2> relative_camera_positions;

		for(const view_index& target_idx : all_vws) {			
			feature_points target_fpoints = feature_points_for_view(ref_cors, target_idx, false);
	
			vec2 target_camera_position_sum = 0.0;
			real target_camera_position_weight_sum = 0.0;
						
			for(const auto& kv : target_fpoints.points) {
				const std::string& feature_name = kv.first;
				const feature_point& target_fpoint = kv.second;
				const feature_point& reference_fpoint = ref_fpoints.points.at(feature_name);
				
				if(! has(j_feature_straight_depths, feature_name)) continue;
				real straight_depth = j_feature_straight_depths.at(feature_name);

				const vec2& target_pos = target_fpoint.position;
				vec2 straight_target_pos = mul_h(unrotate, target_pos);
								
				const vec2& reference_pos = reference_fpoint.position;				
				vec2 straight_reference_pos = mul_h(unrotate, reference_pos);
				
				vec2 feature_target_camera_position;
				feature_target_camera_position[0] = (straight_target_pos[0] - straight_reference_pos[0]) * straight_depth / intr.fx();
				feature_target_camera_position[1] = (straight_target_pos[1] - straight_reference_pos[1]) * straight_depth / intr.fy();
	
				real weight = target_fpoint.weight + reference_fpoint.weight;
				target_camera_position_sum += weight * feature_target_camera_position;
				target_camera_position_weight_sum += weight;
			}
			
			if(target_camera_position_weight_sum > 0.0) {
				vec2 camera_position = target_camera_position_sum / target_camera_position_weight_sum;
				reference_target_camera_positions[ref_idx].emplace_back(target_idx, camera_position);
				target_reference_camera_positions[target_idx].emplace_back(ref_idx, camera_position);
			}
		}
	}
	
	
	std::cout << "computing relative positions of reference views" << std::endl;

	auto reference_camera_displacement = [&](view_index ref_a, view_index ref_b) {
		vec2 displacements_sum = 0.0;
		real displacements_weights_sum = 0.0;
		std::map<view_index, vec2> ref_a_target_camera_positions;
		
		for(const auto& p : reference_target_camera_positions.at(ref_a)) {
			const view_index& target = p.first;
			const vec2& camera_position = p.second;
			ref_a_target_camera_positions[target] = camera_position;
		}
		
		for(const auto& p : reference_target_camera_positions.at(ref_b)) {
			const view_index& target = p.first;
			auto ref_a_pos_it = ref_a_target_camera_positions.find(target);
			if(ref_a_pos_it != ref_a_target_camera_positions.end()) {
				vec2 ref_a_pos = ref_a_pos_it->second;
				vec2 ref_b_pos = p.second;
				
				displacements_sum += (ref_a_pos - ref_b_pos);
				displacements_weights_sum += 1.0;
			}
		}

		if(displacements_weights_sum == 0.0)
			throw std::runtime_error("could not compute displacement from ref " + encode_view_index(ref_a) + " to ref " + encode_view_index(ref_b));
		
		return displacements_sum / displacements_weights_sum;
	};
	
	cors_ref_grid rgrid = references_grid(cors);

	std::map<view_index, vec2> absolute_reference_camera_positions;
	auto add_reference_camera_position = [&](cors_ref_grid_index gi_a, cors_ref_grid_index gi_b) {
		view_index ref_a = rgrid.reference_views.at(gi_a);
		view_index ref_b = rgrid.reference_views.at(gi_b);
		std::cout << "    stitching position of reference view " << ref_b << " onto " << ref_a << std::endl;
		vec2 displacement = reference_camera_displacement(ref_a, ref_b);
		absolute_reference_camera_positions[ref_b] = absolute_reference_camera_positions.at(ref_a) + displacement;
	};
	
	
	absolute_reference_camera_positions[rgrid.reference_views.at(rgrid.center)] = vec2(0.0, 0.0);
	for(int rx = rgrid.center.x - 1; rx >= 0; rx--) {
		add_reference_camera_position({rx+1, rgrid.center.y}, {rx, rgrid.center.y});
		for(int ry = rgrid.center.y - 1; ry >= 0; ry--) add_reference_camera_position({rx, ry+1}, {rx, ry});
		for(int ry = rgrid.center.y + 1; ry < rgrid.count_y; ry++) add_reference_camera_position({rx, ry-1}, {rx, ry});
	}
	for(int rx = rgrid.center.x + 1; rx < rgrid.count_x; rx++) {
		add_reference_camera_position({rx-1, rgrid.center.y}, {rx, rgrid.center.y});
		for(int ry = rgrid.center.y - 1; ry >= 0; ry--) add_reference_camera_position({rx, ry+1}, {rx, ry});
		for(int ry = rgrid.center.y + 1; ry < rgrid.count_y; ry++) add_reference_camera_position({rx, ry-1}, {rx, ry});		
	}
	
	
	std::map<view_index, vec2> absolute_target_camera_positions;
	if(absolute_reference_camera_positions.size() == 1) {
		for(const auto& p : reference_target_camera_positions.begin()->second) {
			absolute_target_camera_positions[p.first] = p.second;
		}
		
	} else {
		std::cout << "stitching camera positions from different reference views" << std::endl;
		for(const view_index& target_index : all_vws) {
			vec2 positions_sum(0.0, 0.0);
			real positions_weights_sum = 0.0;
			
			auto target_positions_it = target_reference_camera_positions.find(target_index);
			if(target_positions_it == target_reference_camera_positions.end()) continue;
			
			for(const auto& p : target_positions_it->second) {
				const view_index& ref_index = p.first;
				const vec2& pos = p.second;
				const vec2& absolute_reference_pos = absolute_reference_camera_positions.at(ref_index);
				real weight = 1.0;
				vec2 abs_pos = absolute_reference_pos + pos;
				positions_sum += abs_pos;
				positions_weights_sum += weight;
			}
			
			absolute_target_camera_positions[target_index] = positions_sum / positions_weights_sum;
		}
		
	}
	

	std::cout << "computing camera array" << std::endl;
	camera_array cameras;
	for(const view_index& target_idx : datas.indices()) {
		auto it = absolute_target_camera_positions.find(target_idx);
		if(it == absolute_target_camera_positions.end()) {
			std::cout << "no camera position for " << target_idx << std::endl;
		}
		const vec2& camera_position = it->second;
		
		camera cam;
		cam.name = datas.view(target_idx).camera_name();
		cam.intrinsic = intr.K;
		cam.rotation = R;
		cam.translation = R * vec3(camera_position[0], camera_position[1], 0.0);
		cameras.push_back(cam);
	}


	std::cout << "saving cameras" << std::endl;
	export_cameras_file(cameras, out_cameras_filename);
	
	std::cout << "done" << std::endl;	
}

