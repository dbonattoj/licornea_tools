#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "lib/cg/references_grid.h"
#include "lib/cg/relative_camera_positions.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <set>
#include <fstream>

using namespace tlz;

const bool verbose = false;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json refgrid.json rcpos.json intr.json R.json out_cameras.json [out_camera_centers.txt]");
	dataset datas = dataset_arg();
	references_grid rgrid = references_grid_arg(); 
	relative_camera_positions rcpos = relative_camera_positions_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	std::string out_cameras_filename = out_filename_arg();
	std::string out_camera_centers_filename = out_filename_opt_arg();
		
	Assert(intr.distortion.is_none(), "input cors + intrinsics must be without distortion");
	
	auto reference_target_camera_positions = rcpos.to_reference_target_positions();
	auto all_target_vws = get_target_views(rcpos);

	std::cout << "computing relative positions of reference views" << std::endl;
	auto reference_camera_displacement = [&](view_index ref_a, view_index ref_b) {
		vec2 displacements_sum = 0.0;
		real displacements_weights_sum = 0.0;
		std::map<view_index, vec2> ref_a_target_camera_positions;
		
		if(reference_target_camera_positions.find(ref_a) != reference_target_camera_positions.end())
		for(const auto& p : reference_target_camera_positions.at(ref_a)) {
			const view_index& target = p.first;
			const vec2& camera_position = p.second;
			ref_a_target_camera_positions[target] = camera_position;
		}
			
		if(reference_target_camera_positions.find(ref_b) != reference_target_camera_positions.end())	
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
	
	
	std::map<view_index, vec2> absolute_reference_camera_positions;
	auto add_reference_camera_position = [&](const view_index& ref_a, const view_index& ref_b) {
		std::cout << "    stitching position of reference view " << ref_b << " onto " << ref_a << std::endl;
		vec2 displacement = reference_camera_displacement(ref_a, ref_b);
		std::cout << "    ref" << ref_b << " = " << displacement << " + ref" << ref_a << std::endl;
		absolute_reference_camera_positions[ref_b] = absolute_reference_camera_positions.at(ref_a) + displacement;
	};
	
	int mid_col = rgrid.cols() / 2, mid_row = rgrid.rows()/2;
	absolute_reference_camera_positions[rgrid.view(mid_col, mid_row)] = vec2(0.0, 0.0);
	for(int col = mid_col-1; col >= 0; col--) {
		add_reference_camera_position(rgrid.view(col+1, mid_row), rgrid.view(col, mid_row));
		for(int row = mid_row-1; row >= 0; row--) add_reference_camera_position(rgrid.view(col, row+1), rgrid.view(col, row));
		for(int row = mid_row+1; row < rgrid.rows(); row++) add_reference_camera_position(rgrid.view(col, row-1), rgrid.view(col, row));
	}
	for(int col = mid_col+1; col < rgrid.cols(); col++) {
		add_reference_camera_position(rgrid.view(col-1, mid_row), rgrid.view(col, mid_row));
		for(int row = mid_row-1; row >= 0; row--) add_reference_camera_position(rgrid.view(col, row+1), rgrid.view(col, row));
		for(int row = mid_row+1; row < rgrid.rows(); row++) add_reference_camera_position(rgrid.view(col, row-1), rgrid.view(col, row));		
	}
	
	
	std::ofstream out_camera_centers_stream;
	if(! out_camera_centers_filename.empty()) {
		out_camera_centers_stream.open(out_camera_centers_filename);
		out_camera_centers_stream << "x y idx_x idx_y chosen\n";
		out_camera_centers_stream << std::setprecision(10);
	}

	std::map<view_index, vec2> absolute_target_camera_positions;
	if(absolute_reference_camera_positions.size() == 1) {
		for(const auto& p : reference_target_camera_positions.begin()->second) {
			const view_index& target_idx = p.first;
			const vec2& pos = p.second;
			absolute_target_camera_positions[p.first] = pos;
			if(out_camera_centers_stream.is_open())
				out_camera_centers_stream << pos[0] << ' ' << pos[1] << ' ' << target_idx.x << ' ' << target_idx.y << " 1\n";
		}
		
	} else {
		reference_target_camera_positions.clear();
		auto target_reference_camera_positions = rcpos.to_target_reference_positions();
		
		std::vector<real> overlap_radii;
		
		std::cout << "stitching camera positions from different reference views" << std::endl;
		for(const view_index& target_idx : all_target_vws) {
			auto target_positions_it = target_reference_camera_positions.find(target_idx);
			if(target_positions_it == target_reference_camera_positions.end()) continue;
			
			struct sample {
				vec2 position;
				view_index target_idx;
				view_index ref_idx;
				
				sample(const vec2& pos, const view_index& tg, const view_index& rf) :
					position(pos), target_idx(tg), ref_idx(rf) { }
				
				int idx_dist() const {
					return sq(ref_idx.x - target_idx.x) + sq(ref_idx.y - target_idx.y);
				}
			};
			std::ptrdiff_t chosen_sample_i = -1;
			std::vector<sample> samples;
			
			for(const auto& p : target_positions_it->second) {
				const view_index& ref_idx = p.first;
				
				const vec2& pos = p.second;
				if(absolute_reference_camera_positions.find(ref_idx) == absolute_reference_camera_positions.end()) continue;
				const vec2& absolute_reference_pos = absolute_reference_camera_positions.at(ref_idx);

				vec2 abs_pos = absolute_reference_pos + pos;
				
				samples.emplace_back(abs_pos, target_idx, ref_idx);
				
				if(chosen_sample_i == -1) chosen_sample_i = samples.size()-1;
				else if(samples.back().idx_dist() < samples.at(chosen_sample_i).idx_dist()) chosen_sample_i = samples.size()-1;		
			}

			for(const sample& samp : samples) {
				bool chosen = (&samp == &samples.at(chosen_sample_i));
				if(chosen)
					absolute_target_camera_positions[samp.target_idx] = samp.position;

				if(out_camera_centers_stream.is_open())
					out_camera_centers_stream << samp.position[0] << ' ' << samp.position[1] << ' ' << samp.target_idx.x << ' ' << samp.target_idx.y << ' ' << (chosen ? '1' : '0') << '\n';
			}

			if(samples.size() > 1) {
				vec2 mean(0.0, 0.0);
				for(const sample& samp : samples) mean += samp.position;
				mean /= real(samples.size());
				real max_dist = 0;
				for(const sample& samp : samples) max_dist = std::max(max_dist, cv::norm(samp.position, mean));
				overlap_radii.push_back(max_dist);
			}
		}
		
		real overlap_radii_avg = 0.0;
		for(real overlap_radius : overlap_radii) overlap_radii_avg += overlap_radius;
		overlap_radii_avg /= overlap_radii.size();
		std::sort(overlap_radii.begin(), overlap_radii.end());
		std::cout << "    overlapping positions (more = better): " << overlap_radii.size() << std::endl;
		std::cout << "    average radius (smaller = better): " << overlap_radii_avg << std::endl;
		std::cout << "    median radius: " << overlap_radii[overlap_radii.size()/2] << std::endl;
		std::cout << "    maximum radius: " << overlap_radii.back() << std::endl;
	}
	

	std::cout << "computing camera array" << std::endl;
	camera_array cameras;
	auto really_all_target_views = datas.indices();
	for(const view_index& target_idx : really_all_target_views) {
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

