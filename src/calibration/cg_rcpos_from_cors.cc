#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "lib/feature_points.h"
#include "lib/image_correspondence.h"
#include "lib/cg/relative_camera_positions.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <set>

using namespace tlz;

const bool verbose = false;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json intr.json R.json straight_depths.json out_rcpos.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	json j_feature_straight_depths = json_arg();
	std::string out_rcpos_filename = out_filename_arg();
	
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	auto all_vws = get_all_views(cors);
	auto ref_vws = get_reference_views(cors);
	
	std::cout << "estimating target camera positions, from each reference" << std::endl;
	relative_camera_positions out_rcpos;		
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
				out_rcpos.position(ref_idx, target_idx) = camera_position;
			}
		}
	}
	
	
	std::cout << "saving relative camera positions" << std::endl;
	export_json_file(encode_relative_camera_positions(out_rcpos), out_rcpos_filename);
}

