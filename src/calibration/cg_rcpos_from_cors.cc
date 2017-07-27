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
#include <algorithm>
#include <fstream>
#include <string>

using namespace tlz;

constexpr bool verbose = false;
constexpr std::size_t minimal_samples_count = 2;
constexpr real maximal_position_stddev = 0.6;

constexpr bool find_bad_features = true;
constexpr real features_inlier_percentile = 0.3;

constexpr bool print_all_sample_positions = false;

struct target_camera_position_result {
	vec2 position = vec2(0.0, 0.0);
	real variance = NAN;
	
	explicit operator bool () const { return ! std::isnan(variance); }
};
target_camera_position_result compute_target_camera_position(const std::map<std::string, vec2>& samples) {	
	if(samples.size() < minimal_samples_count) return target_camera_position_result();
	
	vec2 mean(0.0, 0.0);
	real count = samples.size();
	for(const auto& kv : samples) mean += kv.second;
	mean /= count;
	
	real variance = 0.0;
	for(const auto& kv : samples) {
		const vec2& pos = kv.second;
		variance += sq(pos[0] - mean[0]) + sq(pos[1] - mean[1]);
	}
	variance /= count;
	real stddev = std::sqrt(variance);
	if(stddev > maximal_position_stddev) return target_camera_position_result();
	
	return target_camera_position_result{mean, variance};
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json intr.json R.json straight_depths.json out_rcpos.json [sample_positions.txt] [final_positions.txt]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	straight_depths depths = straight_depths_arg();
	std::string out_rcpos_filename = out_filename_arg();
	std::string out_sample_positions_filename = out_filename_opt_arg();
	std::string out_final_positions_filename = out_filename_opt_arg();
		
	Assert(datas.is_2d(), "need 2d dataset");
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	mat33 unrotate = intr.K * R.t() * intr.K_inv;
	
	std::cout << "collecting all view indices and all reference view indices" << std::endl;
	auto all_vws = get_all_views(cors);
	auto ref_vws = get_reference_views(cors);
	
	view_index ref2 = *(++ref_vws.begin());
	
	using target_camera_position_samples = std::map<std::string, vec2>;
	auto get_target_camera_position_samples = [&](
		const view_index& target_idx,
		const view_index& ref_idx,
		const image_correspondences& ref_cors,
		const feature_points& ref_fpoints
	) -> target_camera_position_samples
	{
		feature_points target_fpoints = feature_points_for_view(ref_cors, target_idx, false);
	
		target_camera_position_samples samples;
						
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
		
			samples[shrt_feature_name] = feature_target_camera_pos;
		}
	
		return samples;
	};

	
	std::set<std::string> bad_features;
	if(find_bad_features) {
		std::cout << "finding bad features" << std::endl;
		struct feature_evalation {
			real average_error = 0.0;
			int samples_count = 0;
		};
		std::map<std::string, feature_evalation> feature_evaluations;
		
		#pragma omp parallel for
		for(std::ptrdiff_t ref_ifx_i = 0; ref_ifx_i < ref_vws.size(); ++ref_ifx_i) {
			const view_index& ref_idx = ref_vws[ref_ifx_i];
			#pragma omp critical
			std::cout << "   reference view " << ref_idx << std::endl;
			
			image_correspondences ref_cors = image_correspondences_with_reference(cors, ref_idx);
			feature_points ref_fpoints = feature_points_for_view(ref_cors, ref_idx, false);
			for(const view_index& target_idx : all_vws) {			
				if(target_idx == ref_idx) continue;
				target_camera_position_samples samples = get_target_camera_position_samples(target_idx, ref_idx, ref_cors, ref_fpoints);
				vec2 mean = 0.0;
				for(const auto& kv : samples) mean += kv.second;
				mean /= real(samples.size());
				for(const auto& kv : samples) {
					const std::string& shrt_feature_name = kv.first;
					const vec2& pos = kv.second;
					real mean_sq_dist = sq(mean[0] - pos[0]) + sq(mean[1] - pos[1]);
					#pragma omp critical
					{
						feature_evaluations[shrt_feature_name].samples_count++;
						feature_evaluations[shrt_feature_name].average_error += mean_sq_dist;
					}
				}
			}
		}
		
		std::vector<std::string> worst_features;
		for(auto& kv : feature_evaluations) {
			const std::string& shrt_feature_name = kv.first;
			feature_evalation& eval = kv.second;
			eval.average_error /= eval.samples_count;
			worst_features.push_back(shrt_feature_name);
		}
		std::ptrdiff_t bad_count = worst_features.size() * (1.0 - features_inlier_percentile);
		auto worst_features_bad_end = worst_features.begin() + bad_count;
		std::partial_sort(
			worst_features.begin(),
			worst_features.begin() + bad_count,
			worst_features.end(),
			[&feature_evaluations](const std::string& a, const std::string& b) {
				real err_a = feature_evaluations.at(a).average_error;
				real err_b = feature_evaluations.at(b).average_error;
				return (err_a > err_b);
			}
		);
		bad_features = std::set<std::string>(worst_features.begin(), worst_features_bad_end);
		std::cout << "   bad features:" << std::endl;
		for(const std::string& shrt_feature_name : bad_features) {
			real err = feature_evaluations.at(shrt_feature_name).average_error;
			std::cout << "      " << shrt_feature_name << " (avg error: " << err << ")" << std::endl;
		}
	}
	
	std::cout << "estimating target camera positions, from each reference" << std::endl;
	relative_camera_positions out_rcpos;		
	
	std::ofstream out_sample_positions_stream, out_final_positions_stream;
	if(! out_sample_positions_filename.empty()) {
		out_sample_positions_stream.open(out_sample_positions_filename);
		out_sample_positions_stream << "x y feature_name target_idx_x target_idx_y ref_idx_x ref_idx_y\n"; 
		out_sample_positions_stream << std::setprecision(10);
	}
	if(! out_final_positions_filename.empty()) {
		out_final_positions_stream.open(out_final_positions_filename);
		out_final_positions_stream << "x y target_idx_x target_idx_y ref_idx_x ref_idx_y\n"; 
		out_final_positions_stream << std::setprecision(10);
	}

	#pragma omp parallel for
	for(std::ptrdiff_t ref_ifx_i = 0; ref_ifx_i < ref_vws.size(); ++ref_ifx_i) {
		const view_index& ref_idx = ref_vws[ref_ifx_i];

		int final_relative_views_count = 0;
			
		if(ref_idx != ref2) continue;
		
		image_correspondences ref_cors = image_correspondences_with_reference(cors, ref_idx);
		feature_points ref_fpoints = feature_points_for_view(ref_cors, ref_idx, false);

		std::map<view_index, vec2> relative_camera_positions;

		for(const view_index& target_idx : all_vws) {
			target_camera_position_result final_pos;
			target_camera_position_samples samples;
			
			if(target_idx == ref_idx) {
				final_pos.position = vec2(0.0, 0.0);
				final_pos.variance = 0.0;
				
			} else {
				samples = get_target_camera_position_samples(target_idx, ref_idx, ref_cors, ref_fpoints);
				if(find_bad_features)
					for(const std::string& shrt_bad_feature_name : bad_features) samples.erase(shrt_bad_feature_name);
			
				final_pos = compute_target_camera_position(samples);
			}

			if(final_pos) {
				++final_relative_views_count;

				#pragma omp critical
				{
					out_rcpos.position(ref_idx, target_idx) = final_pos.position;
	
					if(out_final_positions_stream.is_open())
						out_final_positions_stream << 
							final_pos.position[0] << ' ' <<
							final_pos.position[1] << ' ' <<
							target_idx.x << ' ' <<
							target_idx.y << ' ' <<
							ref_idx.x << ' ' <<
							ref_idx.y << '\n';
				
					for(const auto& kv : samples) {
						const std::string& shrt_feature_name = kv.first;
						const vec2& pos = kv.second;
						bool print_sample = print_all_sample_positions || std::abs(std::abs(target_idx.x - ref_idx.x) - std::abs(target_idx.y - ref_idx.y)) < 4;
						if(out_sample_positions_stream.is_open() && print_sample)
							out_sample_positions_stream <<
								pos[0] << ' ' <<
								pos[1] << ' ' <<
								shrt_feature_name.substr(4) << ' ' <<
								target_idx.x << ' ' <<
								ref_idx.x << ' ' <<
								ref_idx.y << '\n';
					}
				}
			}
		}
		
		#pragma omp critical
		std::cout << "      reference view " << ref_idx << ": final relative positions for " << final_relative_views_count << " views" << std::endl;
	}
	
		
	std::cout << "saving relative camera positions" << std::endl;
	export_json_file(encode_relative_camera_positions(out_rcpos), out_rcpos_filename);
}

