#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "lib/kinect_reprojection_parameters.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: calibrate_color_ir_reprojection obj_img_cors_set.json color_intrinsics.json ir_intrinsics.json out_reprojection_params.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string obj_img_cors_set_filename = argv[1];
	std::string color_intrinsics_filename = argv[2];
	std::string ir_intrinsics_filename = argv[3];
	std::string out_reprojection_parameters_filename = argv[4];
	
	std::cout << "loading obj-img correspondences set" << std::endl;
	auto cors_set = decode_obj_img_correspondences_set<1, 2>(import_json_file(obj_img_cors_set_filename));
	
	std::cout << "loading intrinsics" << std::endl;
	intrinsics color_intr = decode_intrinsics(import_json_file(color_intrinsics_filename));
	intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
	
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
		cv::noArray()
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

