#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/obj_img_correspondence.h"
#include "lib/live/checkerboard.h"
#include <string>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "chk_samples.json cols rows square_width ir_intr.json out_stat.txt");
	std::string chk_samples_filename = in_filename_arg();
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	intrinsics ir_intr = intrinsics_arg();
	std::string out_stat_filename = out_filename_arg();
	
	struct checkerboard_sample {
		std::vector<vec2> corners;
		std::vector<checkerboard_pixel_depth_sample> pixel_samples;
	};
	std::vector<checkerboard_sample> chk_samples;
	
	std::cout << "loading checkerboard samples" << std::endl;
	{	
		json j_chk_samples = import_json_file(chk_samples_filename);
		for(const auto& j_chk_sample : j_chk_samples) {
			checkerboard_sample chk_samp;
			
			const json& j_corners = j_chk_sample["corners"];
			if(j_corners.size() != cols*rows) throw std::runtime_error("wrong checkerboard sample corner count");
			for(const auto& j_corner : j_corners) {
				chk_samp.corners.emplace_back(j_corner["x"], j_corner["y"]);
			}
			const json& j_pixels = j_chk_sample["pixels"];
			for(const auto& j_pixel : j_pixels) {
				checkerboard_pixel_depth_sample pix;
				pix.coordinates = vec2(j_pixel["x"], j_pixel["y"]);
				pix.measured_depth = j_pixel["d"];
				chk_samp.pixel_samples.push_back(pix);
			}
			
			chk_samples.push_back(chk_samp);
		}
	}
	

	struct point_sample {
		real x;
		real y;
		real measured_depth;
		real calculated_depth;
				
		real chk_reprojection_error;
		
		real dist_x;
		real dist_y;		
		
		vec3 chk_rotation;
		vec3 chk_translation;
	};
	std::vector<point_sample> point_samples;

	
	std::vector<vec2> all_distorted_coordinates;
	std::vector<vec2> all_undistorted_coordinates;
	
	
	std::cout << "processing checkerboards (calculate projected distances, reprojection errors)" << std::endl;
	std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);
	for(checkerboard_sample& chk_samp : chk_samples) {
		checkerboard ir_chk(cols, rows, square_width, chk_samp.corners);
	
		checkerboard_extrinsics ext = estimate_checkerboard_extrinsics(ir_chk, ir_intr);
		calculate_checkerboard_pixel_depths(ir_intr, ext, chk_samp.pixel_samples);
		real reprojection_error = checkerboard_reprojection_error(ir_chk, ir_intr, ext);		

		// store point samples for corners
		for(const auto& pix : chk_samp.pixel_samples) {
			point_sample s;
			s.dist_x = pix.coordinates[0];
			s.dist_y = pix.coordinates[1];
			s.measured_depth = pix.measured_depth;
			s.calculated_depth = pix.calculated_depth;
			s.chk_reprojection_error = reprojection_error;
			s.chk_rotation = ext.rotation_vec;
			s.chk_translation = ext.translation;
			point_samples.push_back(s);
			
			all_distorted_coordinates.push_back(pix.coordinates);
		}
	}
	
	
	std::cout << "calculating undistorted points" << std::endl;
	all_undistorted_coordinates.reserve(all_distorted_coordinates.size());
	cv::undistortPoints(
		all_distorted_coordinates,
		all_undistorted_coordinates,
		ir_intr.K,
		ir_intr.distortion.cv_coeffs(),
		cv::noArray(),
		ir_intr.K
	);
	auto samp_it = point_samples.begin();
	for(const vec2& undist : all_undistorted_coordinates) {
		point_sample& s = *(samp_it++);
		s.x = undist[0];
		s.y = undist[1];
	}
	

	std::cout << "saving collected pixel depth samples" << std::endl;
	{
		std::ofstream stream(out_stat_filename);
		stream << "x y measured calculated difference chk_reprojection_err dist_x dist_y chk_rot_x chk_rot_y chk_rot_z chk_t_x chk_t_y chk_t_z\n";
		stream << std::setprecision(10);
		for(const point_sample& samp : point_samples)
			stream
				<< samp.x << " "
				<< samp.y << " "
				<< samp.measured_depth << " "
				<< samp.calculated_depth << " "
				<< samp.measured_depth-samp.calculated_depth << " "
				<< samp.chk_reprojection_error << " "
				<< samp.dist_x << " "
				<< samp.dist_y << " "
				<< samp.chk_rotation[0] << " "
				<< samp.chk_rotation[1] << " "
				<< samp.chk_rotation[2] << " "
				<< samp.chk_translation[0] << " "
				<< samp.chk_translation[1] << " "
				<< samp.chk_translation[2] << "\n";
	}
	
	std::cout << "done" << std::endl;
}







	/*
	 * 
	 * conv from old format:
	 * 
	 * 
	 * 
	
	
	
	std::ifstream old_stream(argv[1]);
	std::ofstream new_stream(argv[2]);
	intrinsics ir_intr = decode_intrinsics(import_json_file(argv[3]));

	std::string line;
	std::getline(old_stream, line);
	using sample = std::array<real, 12>;	
	std::vector<vec2> all_distorted_coordinates;
	std::vector<sample> samples;
	
	while(! old_stream.eof()) {
		sample s;
		for(int i = 0; i < 12; ++i) old_stream >> s[i];
		all_distorted_coordinates.emplace_back(s[0], s[1]);
		samples.push_back(s);
	}
	
	std::vector<vec2> all_undistorted_coordinates;
	all_undistorted_coordinates.reserve(all_distorted_coordinates.size());
	cv::undistortPoints(
		all_distorted_coordinates,
		all_undistorted_coordinates,
		ir_intr.K,
		ir_intr.distortion.cv_coeffs(),
		cv::noArray(),
		ir_intr.K
	);

	std::ptrdiff_t idx = 0;
	new_stream << "x y measured calculated difference chk_reprojection_err dist_x dist_y chk_rot_x chk_rot_y chk_rot_z chk_t_x chk_t_y chk_t_z\n";
	for(const sample& s : samples) {
		new_stream
			<< all_undistorted_coordinates[idx][0] << " "
			<< all_undistorted_coordinates[idx][1] << " "
			<< s[2] << " "
			<< s[3] << " "
			<< s[4] << " "
			<< s[5] << " "
			<< s[0] << " "
			<< s[1] << " "
			<< s[6] << " "
			<< s[7] << " "
			<< s[8] << " "
			<< s[9] << " "
			<< s[10] << " "
			<< s[11] << "\n";
		++idx;
	}
	
	
	return 0;
	*/
