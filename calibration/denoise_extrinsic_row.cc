#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "../../lib/camera.h"
#include "../../lib/eigen.h"

[[noreturn]] void usage_fail() {
	std::cout << "usage: denoise_extrinsic_row in_cameras.json out_cameras.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string in_cameras_filename = argv[1];
	std::string out_cameras_filename = argv[2];
	
	auto cameras = read_cameras_file(in_cameras_filename);
	
	std::vector<Eigen_mat4> inv_extrinsics;
	for(const camera& cam : cameras) inv_extrinsics.push_back(cam.extrinsic.inverse());
	

	Eigen_vec3 min_trans = inv_extrinsics.front().block<3, 1>(0, 3);
	Eigen_vec3 max_trans = inv_extrinsics.back().block<3, 1>(0, 3);
	

	int count = cameras.size();
	for(int i = 0; i < count; ++i) {
		double k = (double)i / (count - 1);
		inv_extrinsics[i].block<3, 1>(0, 3) = (1.0-k)*min_trans + k*max_trans;
		cameras[i].extrinsic = inv_extrinsics[i].inverse();
	}
	
	write_cameras_file(out_cameras_filename, cameras);
}

