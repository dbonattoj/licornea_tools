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
	get_args(argc, argv,
		"dataset_parameters.json cors.json intr.json R.json straight_depths.json out_cameras.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	json j_feature_straight_depths = json_arg();
	std::string out_cameras_filename = out_filename_arg();
	
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	auto all_vws = all_views(cors);
	auto ref_vws = reference_views(cors); // TODO rename functions
	
	using reference_target_key = std::pair<view_index, view_index>;
	auto reference = [](const reference_target_key& key) { return key.first; };
	auto target = [](const reference_target_key& key) { return key.second; };
	
	std::map<reference_target_key, vec2> reference_target_camera_positions;
	
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
				real straight_depth = j_feature_straight_depths[feature_name];

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
			
			if(target_camera_position_weight_sum > 0.0)
				reference_target_camera_positions[reference_target_key(ref_idx, target_idx)] = target_camera_position_sum / target_camera_position_weight_sum;
		}
	}
	
	

	
	std::vector<int> ref_x_positions, ref_y_positions;
	{
		std::set<int> ref_x_positions_set, ref_y_positions_set;
		for(const view_index& idx : ref_vws) {
			ref_x_positions_set.insert(idx.x);
			ref_y_positions_set.insert(idx.y);
		}
		for(int x : ref_x_positions_set) ref_x_positions.push_back(x);
		for(int y : ref_y_positions_set) ref_y_positions.push_back(y);
	}
	std::size_t ref_grid_x_size = ref_x_positions.size();
	std::size_t ref_grid_y_size = ref_y_positions.size();
	
	
	auto get_ref_idx = [&](int gx, int gy) {
		return view_index(ref_x_positions.at(gx), ref_y_positions.at(gy));
	};
	for(int gx = 0; gx < ref_grid_x_size.size(); ++gx)
	for(int gy = 0; gy < ref_grid_y_size.size(); ++gy) {
		view_index ref_idx = get_ref_idx(gx, gy);
		if(reference_target_camera_positions.find(ref_idx) == reference_target_camera_positions.end())
			throw std::runtime_error("reference views are not arranged in a grid");
	}
	
	std::map<view_index, vec2> absolute_target_camera_positions;
	
	std::map<view_index, vec2> absolute_reference_camera_positions;

	auto reference_camera_displacement = [&](view_index ref_a, view_index ref_b) {
		vec2 displacements_sum = 0.0;
		real displacements_weights_sum = 0.0;
		std::set<view_index> ref_a_targets;
		
		for(const auto& kv : reference_target_camera_positions) {
			const view_index& ref = kv.first.first;
			const view_index& target = kv.first.second;
			if(ref == ref_a) ref_a_targets.insert(target);
		}
		
		for(const auto& kv : reference_target_camera_positions) {
			const view_index& ref = kv.first.first;
			const view_index& target = kv.first.second;
			if(ref == ref_b && ref_a_targets.find(target) != ref_a_targets.end()) {
				vec2 ref_a_pos = reference_target_camera_positions.at(reference_target_key(ref_a, target));
				vec2 ref_b_pos = reference_target_camera_positions.at(reference_target_key(ref_b, target));
				
				displacements_sum += (ref_b_pos - ref_a_pos);
				displacements_weights_sim += 1.0;
			}
		}
		
		if(displacements_weights_sum == 0.0)
			throw std::runtime_error("could not compute displacement from ref " + encode_view_index(ref_a) + " to ref " + encode_view_index(ref_b));
		
		return displacements_sum / displacements_weights_sum;
	};
	
	auto add_reference_camera_position = [&](view_index ref_a, view_index ref_b) {
		vec2 displacement = reference_camera_displacement(ref_a, ref_b);
		absolute_reference_camera_positions[ref_b] = absolute_reference_camera_positions.at(ref_a) + displacement;
	};
	

	int root_rx = ref_grid_x_size/2, root_ry = ref_grid_y_size/2;
	absolute_reference_camera_positions[get_ref_idx(root_rx, root_ry)] = vec2(0.0, 0.0);
	for(int rx = root_rx - 1; rx >= 0; rx--) {
		add_reference_camera_position(get_ref_idx(rx+1, root_ry), get_ref_idx(rx, root_ry));
		for(ry = root_ry - 1; ry >= 0; ry--) add_reference_camera_position(get_ref_idx(rx, ry+1), get_ref_idx(rx, ry));
		for(ry = root_ry + 1; ry < ref_grid_y_size; ry++) add_reference_camera_position(get_ref_idx(rx, ry-1), get_ref_idx(rx, ry));
	}
	for(int rx = root_rx + 1; rx < ref_grid_x_size; rx++) {
		add_reference_camera_position(get_ref_idx(rx-1, root_ry), get_ref_idx(rx, root_ry));
		for(ry = root_ry - 1; ry >= 0; ry--) add_reference_camera_position(get_ref_idx(rx, ry+1), get_ref_idx(rx, ry));
		for(ry = root_ry + 1; ry < ref_grid_y_size; ry++) add_reference_camera_position(get_ref_idx(rx, ry-1), get_ref_idx(rx, ry));		
	}
	
	
/*
	std::map<view_index, vec2> absolute_target_camera_positions;
	const view_index& first_ref = *ref_vws.begin();
	for(const auto& kv : reference_target_camera_positions) {
		if(reference(kv.first) == first_ref)
			absolute_target_camera_positions[target(kv.first)] = kv.second;
	}
*/
	
	
	
	camera_array cameras;
	for(const view_index& target_idx : all_vws) {
		auto it = absolute_target_camera_positions.find(target_idx);
		if(it == absolute_target_camera_positions.end()) continue;
		const vec2& camera_position = it->second;
		
		camera cam;
		cam.name = datas.view(target_idx).camera_name();
		cam.intrinsic = intr.K;
		cam.rotation = R;
		cam.translation = R * vec3(camera_position[0], camera_position[1], 0.0);
		cameras.push_back(cam);
	}


	std::cout << "saving cameras" << std::endl;
	write_cameras_file(out_cameras_filename, cameras);
	
	std::cout << "done" << std::endl;

	
	
	/*
	std::map<std::pair<view_index, view_index>, vec2> reference_to_reference_camera_distances;
	for(auto ref1_it = ref_views.begin(); ref1_it != ref_views.end(); ++ref1_it)
	for(auto ref2_it = ref1_it; ref2_it != ref_views.end(); ++ref2_it) {
		auto it_1to2 = reference_target_camera_positions.find(reference_target_key(*ref1_it, *ref2_it));
		auto it_2to1 = reference_target_camera_positions.find(reference_target_key(*ref2_it, *ref1_it));
		auto end = reference_target_camera_positions.end();

		auto key = std::make_pair(*ref1_it, *ref2_it);

		if(it_1to2 != end && it_2to1 != end)
			reference_to_reference_camera_distances[key] = (*it_1to2 - *it_2to1) / 2.0;
		else if(it_1to2 != end) {
			reference_to_reference_camera_distances[key] = *it_1to2;
		else if(it_2to1 != end) {
			reference_to_reference_camera_distances[key] = -*it_2to1;
	}
	
	

	
	
	for(const view_index& ref1_idx : ref_views) for(const view_index& ref2_idx : ref_views) {
		reference_target_key key(ref1_idx, ref2_idx);
		auto it = reference_target_camera_positions.find(key);
		if(it != reference_target_camera_positions.end())
			reference_to_reference_camera_distances[key] = *it;
	}
	*/
	
	
	
}

