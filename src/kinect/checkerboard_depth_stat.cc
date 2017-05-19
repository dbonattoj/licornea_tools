#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "../lib/obj_img_correspondence.h"
#include "lib/live/checkerboard.h"
#include <string>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_depth_stat chk_samples.json cols rows square_width ir_intr.json out_stat.txt" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 6) usage_fail();
	std::string chk_samples_filename = argv[1];
	int cols = std::atoi(argv[2]);
	int rows = std::atoi(argv[3]);
	real square_width = std::atof(argv[4]);
	std::string ir_intrinsics_filename = argv[5];
	std::string out_stat_filename = argv[6];
		
	std::cout << "loading intrinsics" << std::endl;
	intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
	
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
		
		vec3 chk_rotation;
		vec3 chk_translation;
	};
	std::vector<point_sample> point_samples;

	
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
			s.x = pix.coordinates[0];
			s.y = pix.coordinates[1];
			s.measured_depth = pix.measured_depth;
			s.calculated_depth = pix.calculated_depth;
			s.chk_reprojection_error = reprojection_error;
			s.chk_rotation = ext.rotation_vec;
			s.chk_translation = ext.translation;
			point_samples.push_back(s);
		}
	}
	

	std::cout << "saving collected point samples" << std::endl;
	{
		std::ofstream stream(out_stat_filename);
		stream << "x y measured calculated difference chk_reprojection_err chk_rot_x chk_rot_y chk_rot_z chk_t_x chk_t_y chk_t_z\n";
		for(const point_sample& samp : point_samples)
			stream
				<< samp.x << " "
				<< samp.y << " "
				<< samp.measured_depth << " "
				<< samp.calculated_depth << " "
				<< samp.measured_depth-samp.calculated_depth << " "
				<< samp.chk_reprojection_error << " "
				<< samp.chk_rotation[0] << " "
				<< samp.chk_rotation[1] << " "
				<< samp.chk_rotation[2] << " "
				<< samp.chk_translation[0] << " "
				<< samp.chk_translation[1] << " "
				<< samp.chk_translation[2] << "\n";
	}
	
	std::cout << "done" << std::endl;
}
