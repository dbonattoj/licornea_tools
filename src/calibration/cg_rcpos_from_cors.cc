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
#include "lib/cg/straight_depths.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <set>

#include <fstream>

using namespace tlz;

const bool verbose = false;

struct target_camera_position_sample {
	vec2 pos;
	real w;
};


vec2 compute_target_camera_position(std::vector<target_camera_position_sample>& samples) {
	if(samples.size() < 1) return vec2();
	
	vec2 sum(0.0, 0.0);
	real weight_sum = 0.0;
	for(const auto& samp : samples) {
		sum += samp.w * samp.pos;
		weight_sum += samp.w;
	}
	vec2 mean = sum / weight_sum;
	//return mean;
	///*
	
	auto mag = [&mean](const target_camera_position_sample& position) {
		return sq(position.pos[0] + mean[0]) + sq(position.pos[1] + mean[1]);
	};
	auto cmp = [&mag](const target_camera_position_sample& a, const target_camera_position_sample& b) {
		return (mag(a) < mag(b));
	};
	std::sort(
		samples.begin(),
		samples.end(),
		cmp
	);

	const real percentile = 0.9;
	const std::ptrdiff_t margin = samples.size() * ((1.0 - percentile)/2.0);
	auto begin_it = samples.begin() + margin;
	auto end_it = samples.end() - margin;
	sum = vec2(0.0, 0.0);
	weight_sum = 0.0;
	for(auto it = begin_it; it != end_it; ++it) {
		const auto& samp = *it;
		sum += samp.w * samp.pos;
		weight_sum += samp.w;
	}
	mean = sum / weight_sum;
	return mean;
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json intr.json R.json straight_depths.json out_rcpos.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	straight_depths depths = straight_depths_arg();
	std::string out_rcpos_filename = out_filename_arg();
	
	std::ofstream out_file("positions.txt");
	std::ofstream out_final_file("final_positions.txt");
	
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
			if(target_idx == ref_idx) {
				out_rcpos.position(ref_idx, target_idx) = vec2(0.0, 0.0);
				continue;
			}
					
			feature_points target_fpoints = feature_points_for_view(ref_cors, target_idx, false);
	
			std::vector<target_camera_position_sample> target_camera_position_samples;
			target_camera_position_samples.reserve(target_fpoints.points.size());
							
			for(const auto& kv : target_fpoints.points) {
				const std::string& feature_name = kv.first;
				const feature_point& target_fpoint = kv.second;
				const feature_point& reference_fpoint = ref_fpoints.points.at(feature_name);
				
				if(depths.find(feature_name) == depths.end()) continue;
				real straight_depth = depths.at(feature_name).depth;

				const vec2& target_pos = target_fpoint.position;
				vec2 straight_target_pos = mul_h(unrotate, target_pos);
								
				const vec2& reference_pos = reference_fpoint.position;				
				vec2 straight_reference_pos = mul_h(unrotate, reference_pos);
				
				vec2 feature_target_camera_pos;
				feature_target_camera_pos[0] = (straight_target_pos[0] - straight_reference_pos[0]) * straight_depth / intr.fx();
				feature_target_camera_pos[1] = (straight_target_pos[1] - straight_reference_pos[1]) * straight_depth / intr.fy();
	
				//if(target_idx.y % 50 == 0 && target_idx.x % 50 == 0)
				//	out_file << feature_target_camera_pos[0]+(target_idx.x*10) << " " << feature_target_camera_pos[1]+(target_idx.y*10) << " " << feature_name.substr(4) << "\n";
		
				real weight = target_fpoint.weight + reference_fpoint.weight;
				target_camera_position_samples.push_back({ feature_target_camera_pos, weight });
			}
			
			
			vec2 final_pos = compute_target_camera_position(target_camera_position_samples);
			if(final_pos != vec2(0.0, 0.0)) {
				out_rcpos.position(ref_idx, target_idx) = final_pos;

				//if(target_idx.y % 50 == 0 && target_idx.x % 50 == 0)
				//	out_final_file << final_pos[0]+(target_idx.x*10) << " " << final_pos[1]+(target_idx.y*10) << " " << target_idx.x << " " << target_idx.y << "\n";
				//out_final_file << ref_idx.x*3+final_pos[0] << " " << final_pos[1] << " " << target_idx.x << " " << target_idx.y << "\n";
			}
		}
	}
	
	
	std::cout << "saving relative camera positions" << std::endl;
	export_json_file(encode_relative_camera_positions(out_rcpos), out_rcpos_filename);
}

