#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: calibrate_intrinsics obj_img_cors_set.json image_width image_height out_intrinsics.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string obj_img_cors_set_filename = argv[1];
	int image_width = std::atoi(argv[2]);
	int image_height = std::atoi(argv[3]);
	std::string out_intrinsics_filename = argv[4];
	
	std::cout << "loading obj-img correspondences set" << std::endl;
	auto cors_set = decode_obj_img_correspondences_set<1, 1>(import_json_file(obj_img_cors_set_filename));
	
	std::cout << "preparing input data" << std::endl;
	std::vector<std::vector<cv::Vec3f>> object_points(cors_set.size());
	std::vector<std::vector<cv::Vec2f>> image_points(cors_set.size());
	for(int set_i = 0; set_i < cors_set.size(); ++set_i) {
		const auto& cors = cors_set[set_i];
		auto& obj_points = object_points[set_i];
		auto& img_points = image_points[set_i];
		for(const auto& cor : cors) {
			obj_points.push_back(cor.object_coordinates[0]);
			img_points.push_back(cor.image_coordinates[0]);
		}
	}
	
	cv::Size image_size(image_width, image_height);
	
	mat33 out_camera_mat;
	std::vector<real> out_distortion(4);
	
	std::cout << "computing calibration" << std::endl;
	std::vector<cv::Mat> out_rotations, out_translations;
	cv::calibrateCamera(
		object_points,
		image_points,
		image_size,
		out_camera_mat,
		out_distortion,
		out_rotations,
		out_translations,
		0
	);
	
	std::cout << "saving intrinsics" << std::endl;
	intrinsics intr;
	intr.K = out_camera_mat;
	intr.distortion.k1 = out_distortion[0];
	intr.distortion.k2 = out_distortion[1];
	intr.distortion.p1 = out_distortion[2];
	intr.distortion.p2 = out_distortion[3];
	intr.width = image_width;
	intr.height = image_height;
	export_json_file(encode_intrinsics(intr), out_intrinsics_filename);
	
	std::cout << "done" << std::endl;
}

