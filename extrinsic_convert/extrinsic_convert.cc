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

int main(int argc, const char* argv[]) {
	if(argc <= 3) {
		std::cout << "usage: " << argv[0] << " in_cameras.txt out_cameras.txt operation\n";
		std::cout << "operation: Rt2MPEG, MPEG2Rt, flip_t" << std::endl;
		return EXIT_FAILURE;
	}
	std::string in_cameras = argv[1];
	std::string out_cameras = argv[2];
	std::string operation = argv[3];
	
	std::ifstream input(in_cameras.c_str());
	std::ofstream output(out_cameras.c_str());
	
	for(;;) {
		input.exceptions(std::ios_base::badbit);	
		std::string camera_name;
		input >> camera_name;
		if(input.eof() || input.fail()) break;
	
		if(camera_name == "0") { // extra last row (0 0 0 1) of last extrinsic
			int unused;
			input >> unused >> unused >> unused;
			continue;
		}

		Eigen_mat3 intrinsic = Eigen_mat3::Zero();
		input >> intrinsic(0, 0) >> intrinsic(0, 1) >> intrinsic(0, 2);
		input >> intrinsic(1, 0) >> intrinsic(1, 1) >> intrinsic(1, 2);
		input >> intrinsic(2, 0) >> intrinsic(2, 1) >> intrinsic(2, 2);
		
		Eigen_scalar unused[2]; // unused, always 0
		input >> unused[0] >> unused[1];

		Eigen_mat3 rotation;
		Eigen_vec3 translation;
		input >> rotation(0, 0) >> rotation(0, 1) >> rotation(0, 2) >> translation(0);
		input >> rotation(1, 0) >> rotation(1, 1) >> rotation(1, 2) >> translation(1);
		input >> rotation(2, 0) >> rotation(2, 1) >> rotation(2, 2) >> translation(2);
		
		if(operation == "Rt2MPEG") {
			translation = -(rotation.inverse() * translation);
		} else if(operation == "MPEG2Rt") {
			translation = -(rotation * translation);			
		} else if(operation == "flip_t") {
			translation = -translation;
		}
		
		output << camera_name << "\n";
		output << intrinsic(0, 0) << ' ' << intrinsic(0, 1) << ' ' << intrinsic(0, 2) << '\n';
		output << intrinsic(1, 0) << ' ' << intrinsic(1, 1) << ' ' << intrinsic(1, 2) << '\n';
		output << intrinsic(2, 0) << ' ' << intrinsic(2, 1) << ' ' << intrinsic(2, 2) << '\n';
		output << unused[0] << '\n' << unused[1] << '\n';
		output << rotation(0, 0) << ' ' << rotation(0, 1) << ' ' << rotation(0, 2) << ' ' << translation(0) << '\n';
		output << rotation(1, 0) << ' ' << rotation(1, 1) << ' ' << rotation(1, 2) << ' ' << translation(1) << '\n';
		output << rotation(2, 0) << ' ' << rotation(2, 1) << ' ' << rotation(2, 2) << ' ' << translation(2) << '\n';
		output << '\n';
	}

}
