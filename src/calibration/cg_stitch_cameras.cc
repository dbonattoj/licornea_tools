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
	
	
	std::map<view_index, vec2> absolute_target_camera_positions;
	if(absolute_reference_camera_positions.size() == 1) {
		for(const auto& p : reference_target_camera_positions.begin()->second) {
			absolute_target_camera_positions[p.first] = p.second;
		}
		
	} else {
		reference_target_camera_positions.clear();
		auto target_reference_camera_positions = rcpos.to_target_reference_positions();
		
		std::vector<real> overlap_radii;
		
		std::cout << "stitching camera positions from different reference views" << std::endl;
		for(const view_index& target_index : all_target_vws) {
			int min_idx_dist = 0;
			
			auto target_positions_it = target_reference_camera_positions.find(target_index);
			if(target_positions_it == target_reference_camera_positions.end()) continue;
			
			std::vector<vec2> samples;
			
			for(const auto& p : target_positions_it->second) {
				const view_index& ref_index = p.first;
				int dist = sq(ref_index.x - target_index.x) + sq(ref_index.y - target_index.y);
				if(min_idx_dist != 0 && dist >= min_idx_dist) continue;
				
				const vec2& pos = p.second;
				if(absolute_reference_camera_positions.find(ref_index) == absolute_reference_camera_positions.end()) continue;
				const vec2& absolute_reference_pos = absolute_reference_camera_positions.at(ref_index);

				vec2 abs_pos = absolute_reference_pos + pos;
				absolute_target_camera_positions[target_index] = abs_pos;
				min_idx_dist = dist;
				
				samples.push_back(abs_pos);
			}

			if(samples.size() > 1) {
				vec2 mean(0.0, 0.0);
				for(const vec2& samp : samples) mean += samp;
				mean /= real(samples.size());
				real max_dist = 0;
				for(const vec2& samp : samples) max_dist = std::max(max_dist, cv::norm(samp, mean));
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
	for(const view_index& target_idx : all_target_vws) {
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
	
	if(! out_camera_centers_filename.empty()) {
		std::cout << "saving camera center positions" << std::endl;
		std::ofstream out_camera_centers_stream(out_camera_centers_filename);
		out_camera_centers_stream << "center_x center_y idx_x idx_y\n";
		for(const auto& kv : absolute_target_camera_positions) {
			const view_index& target_idx = kv.first;
			const vec2& position = kv.second;
			out_camera_centers_stream << position[0] << ' ' << position[1] << ' ' << target_idx.x << ' ' << target_idx.y << '\n';
		}
	}
	
	std::cout << "done" << std::endl;	
}

