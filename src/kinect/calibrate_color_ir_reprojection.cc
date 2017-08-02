#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "lib/kinect_reprojection_parameters.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <climits>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "obj_img_cors_set.json color_intrinsics.json ir_intrinsics.json out_reprojection_params.json");
	std::string obj_img_cors_set_filename = in_filename_arg();
	intrinsics color_intr = intrinsics_arg();
	intrinsics ir_intr = intrinsics_arg();
	std::string out_reprojection_parameters_filename = out_filename_arg();
	

	std::cout << "loading obj-img correspondences set" << std::endl;
	auto cors_set = decode_obj_img_correspondences_set<1, 2>(import_json_file(obj_img_cors_set_filename));

		
	std::cout << "preparing input data" << std::endl;
	std::vector<std::vector<cv::Vec3f>> object_points(cors_set.size());
	std::vector<std::vector<cv::Vec2f>> color_image_points(cors_set.size());
	std::vector<std::vector<cv::Vec2f>> ir_image_points(cors_set.size());
	for(int set_i = 0; set_i < cors_set.size(); ++set_i) {
		const auto& cors = cors_set[set_i];
		auto& obj_points = object_points[set_i];
		auto& color_img_points = color_image_points[set_i];
		auto& ir_img_points = ir_image_points[set_i];
		for(const auto& cor : cors) {
			obj_points.push_back(cor.object_coordinates[0]);
			color_img_points.push_back(cor.image_coordinates[0]);
			ir_img_points.push_back(cor.image_coordinates[1]);
		}
	}
	
	
	std::cout << "doing stereo calibration" << std::endl;
	mat33 out_rotation;
	vec3 out_translation;
	auto color_distortion_coeffs = color_intr.distortion.cv_coeffs();
	auto ir_distortion_coeffs = ir_intr.distortion.cv_coeffs();
	
	
	cv::TermCriteria term(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 100, DBL_EPSILON);
	real reproj_error = cv::stereoCalibrate(
		object_points,
		color_image_points,
		ir_image_points,
		color_intr.K,
		color_distortion_coeffs,
		ir_intr.K,
		ir_distortion_coeffs,
		cv::Size(1, 1), // ignored
		out_rotation,
		out_translation,
		cv::noArray(),
		cv::noArray(),
		#ifdef HAVE_OPENCV3
		cv::CALIB_FIX_INTRINSIC,
		term 
		#else
		term,
		cv::CALIB_FIX_INTRINSIC
		#endif
	);
	
		
	{
		vec3 rotation_vec;
		cv::Rodrigues(out_rotation, rotation_vec);
	
		std::cout << "reprojection error: " << reproj_error << std::endl;

		std::cout << "translation: " << out_translation << std::endl;
		std::cout << "rotation (degrees): " << rotation_vec * (180.0/3.1415) << std::endl;
	}


	std::cout << "saving reprojection parameters" << std::endl;
	kinect_reprojection_parameters parameters;
	parameters.ir_intrinsics = ir_intr;
	parameters.color_intrinsics = color_intr;
	parameters.rotation = out_rotation;
	parameters.translation = out_translation;
	export_json_file(encode_kinect_reprojection_parameters(parameters), out_reprojection_parameters_filename);


	std::cout << "done" << std::endl;
}

