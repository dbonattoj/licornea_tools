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

constexpr bool verbose = false;
constexpr std::size_t minimal_samples_count = 2;
constexpr real position_inlier_percentile = 0.8;
constexpr real maximal_position_stddev = 0.6;

struct target_camera_position_sample {
	vec2 pos;
	real w;
};


vec2 compute_target_camera_position(std::vector<target_camera_position_sample>& samples) {	
	if(samples.size() < minimal_samples_count) return vec2();
	
	vec2 sum(0.0, 0.0);
	real weight_sum = 0.0;
	for(const auto& samp : samples) {
		sum += samp.w * samp.pos;
		weight_sum += samp.w;
	}
	vec2 mean = sum / weight_sum;

	
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

	const std::ptrdiff_t margin = samples.size() * ((1.0 - position_inlier_percentile)/2.0);
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
	
	real stddev = 0.0;
	for(auto it = begin_it; it != end_it; ++it) {
		const auto& samp = *it;
		stddev += samp.w * cv::norm(samp.pos - mean);
	}
	stddev = std::sqrt(stddev / weight_sum);
	std::cout << stddev << std::endl;
	if(stddev > maximal_position_stddev) return vec2();
	
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
	
	std::ofstream out_file("positions.txt"); out_file << std::setprecision(10);
	std::ofstream out_final_file("final_positions.txt"); out_final_file << std::setprecision(10);
	
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	auto all_vws = get_all_views(cors);
	auto ref_vws = get_reference_views(cors);
	
	view_index ref2 = *(++++++ref_vws.begin());
		
	std::cout << "estimating target camera positions, from each reference" << std::endl;
	relative_camera_positions out_rcpos;		
	for(const view_index& ref_idx : ref_vws) {
		int final_relative_views_count = 0;
		std::cout << "   reference view " << ref_idx << std::endl;
		if(ref_idx != ref2) continue;
		
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
				const std::string& shrt_feature_name = short_feature_name(feature_name);
				const feature_point& target_fpoint = kv.second;

				if(! ref_fpoints.has_feature(feature_name)) continue;
				const feature_point& reference_fpoint = ref_fpoints.points.at(feature_name);

				//if(feature_name != "feat2033" && feature_name != "feat2038" && feature_name != "feat2020") continue;
				
				if(depths.find(shrt_feature_name) == depths.end()) continue;
				real straight_d = depths.at(shrt_feature_name).depth;

				const vec2& target_pos = target_fpoint.position;
				vec2 straight_target_pos = mul_h(unrotate, target_pos);
								
				const vec2& reference_pos = reference_fpoint.position;				
				vec2 straight_reference_pos = mul_h(unrotate, reference_pos);
				
				vec2 feature_target_camera_pos;
				feature_target_camera_pos[0] = (straight_target_pos[0] - straight_reference_pos[0]) * straight_d / intr.fx();
				feature_target_camera_pos[1] = (straight_target_pos[1] - straight_reference_pos[1]) * straight_d / intr.fy();
	
				//if(target_idx.x >= 450 && target_idx.x <= 455 && target_idx.y >= 150 && target_idx.y <= 155)
					out_file << feature_target_camera_pos[0] << " " << feature_target_camera_pos[1] << " " << shrt_feature_name.substr(4) << " " << target_idx.x << " " << target_idx.y << "\n";
		
				real weight = target_fpoint.weight + reference_fpoint.weight;
				target_camera_position_samples.push_back({ feature_target_camera_pos, weight });
			}
			
			
			vec2 final_pos = compute_target_camera_position(target_camera_position_samples);
			if(final_pos != vec2(0.0, 0.0)) {
				out_rcpos.position(ref_idx, target_idx) = final_pos;
				++final_relative_views_count;

				//if(target_idx.x >= 450 && target_idx.x <= 455 && target_idx.y >= 150 && target_idx.y <= 155)
					out_final_file << final_pos[0] << " " << final_pos[1] << " " << target_idx.x << " " << target_idx.y << "\n";
			}		
		}
		
		std::cout << "   relative positions for " << final_relative_views_count << " views" << std::endl;
	}
	
	
	std::cout << "saving relative camera positions" << std::endl;
	export_json_file(encode_relative_camera_positions(out_rcpos), out_rcpos_filename);
}

