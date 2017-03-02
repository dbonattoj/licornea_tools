#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-register"
	#include <Eigen/Eigen>
#pragma GCC diagnostic pop

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using Eigen_scalar = double;
using Eigen_mat3 = Eigen::Matrix3d;
using Eigen_vec3 = Eigen::Vector3d;

struct camera {
	std::string name;
	Eigen_mat3 intrinsic;
	Eigen_mat3 rotation;
	Eigen_vec3 translation;
};

bool read_camera(std::istream& input, camera& cam) {
	input >> cam.name;
	if(input.eof() || input.fail()) return false;
	
	if(cam.name == "0") { // extra last row (0 0 0 1) of previous extrinsic
		int unused;
		input >> unused >> unused >> unused;
		return read_camera(input, cam);
	}

	input >> cam.intrinsic(0, 0) >> cam.intrinsic(0, 1) >> cam.intrinsic(0, 2);
	input >> cam.intrinsic(1, 0) >> cam.intrinsic(1, 1) >> cam.intrinsic(1, 2);
	input >> cam.intrinsic(2, 0) >> cam.intrinsic(2, 1) >> cam.intrinsic(2, 2);
		
	Eigen_scalar unused[2]; // unused, always 0
	input >> unused[0] >> unused[1];

	input >> cam.rotation(0, 0) >> cam.rotation(0, 1) >> cam.rotation(0, 2) >> cam.translation(0);
	input >> cam.rotation(1, 0) >> cam.rotation(1, 1) >> cam.rotation(1, 2) >> cam.translation(1);
	input >> cam.rotation(2, 0) >> cam.rotation(2, 1) >> cam.rotation(2, 2) >> cam.translation(2);
	
	return true;
}

void write_camera(std::ostream& output, const camera& cam) {
	output << cam.name << "\n";
	output << cam.intrinsic(0, 0) << ' ' << cam.intrinsic(0, 1) << ' ' << cam.intrinsic(0, 2) << '\n';
	output << cam.intrinsic(1, 0) << ' ' << cam.intrinsic(1, 1) << ' ' << cam.intrinsic(1, 2) << '\n';
	output << cam.intrinsic(2, 0) << ' ' << cam.intrinsic(2, 1) << ' ' << cam.intrinsic(2, 2) << '\n';
	output << "0\n0\n";
	output << cam.rotation(0, 0) << ' ' << cam.rotation(0, 1) << ' ' << cam.rotation(0, 2) << ' ' << cam.translation(0) << '\n';
	output << cam.rotation(1, 0) << ' ' << cam.rotation(1, 1) << ' ' << cam.rotation(1, 2) << ' ' << cam.translation(1) << '\n';
	output << cam.rotation(2, 0) << ' ' << cam.rotation(2, 1) << ' ' << cam.rotation(2, 2) << ' ' << cam.translation(2) << '\n';
	output << '\n';
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: camera_transform in_cameras.txt out_cameras.txt operation\n";
	std::cout << "operations: Rt2MPEG: standard extrinsic matrix to MPEG convention\n";
	std::cout << "            MPEG2Rt: MPEG convention to standard extrinsic matrix\n";
	std::cout << "            flip_t: flip sign of translation vectors\n";
	std::cout << "            scale old new: adapt intrinsic matrix for image scale from old to new\n";
	std::cout << "                           (old/new = pixel length of same segment in old and new image)\n";
	std::cout << "            nop: No change, just reformat the cameras file\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string in_cameras = argv[1];
	std::string out_cameras = argv[2];
	std::string operation = argv[3];
	
	std::ifstream input(in_cameras.c_str());
	std::ofstream output(out_cameras.c_str());
	input.exceptions(std::ios_base::badbit);	
	
	camera cam;
	
	while(read_camera(input, cam)) {
		if(operation == "Rt2MPEG") {
			cam.translation = -(cam.rotation.inverse() * cam.translation);
			
		} else if(operation == "MPEG2Rt") {
			cam.translation = -(cam.rotation * cam.translation);	
					
		} else if(operation == "flip_t") {
			cam.translation = -cam.translation;			
			
		} else if(operation == "scale") {
			if(argc <= 5) usage_fail();
			int old_sz = std::atoi(argv[4]);
			int new_sz = std::atoi(argv[5]);
			float factor = (float)new_sz / old_sz;
			cam.intrinsic(0,0) *= factor;
			cam.intrinsic(1,1) *= factor;
			cam.intrinsic(0,2) *= factor;
			cam.intrinsic(1,2) *= factor;
			
		} else if(operation == "nop") {
			// no change
		} else {
			usage_fail();
		}
		
		write_camera(output, cam);
	}
	
	std::cout << "done" << std::endl;
}
