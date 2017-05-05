int main(){}
#if 0
#include <string>
#include <map>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/eigen.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cameras_on_line dataset_parameters.json image_correspondences.json intrinsics.json out_cams.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cams_filename = argv[4];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading intrinsic matrix" << std::endl;
	const json& j_intrinsics = import_json_file(intrinsics_filename);
	cv::Mat3d K = decode_mat_cv(j_intrinsics["K"]);
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	
	real yaw = 0.0;
	real sinyaw = std::sin(yaw), cosyaw = std::cos(yaw);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	view_index reference_view_idx = cors.reference;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		for(const auto& kv2 : feature.points) {
			const view_index& idx = kv.first;
			real ix = kv.second[0], iy = kv.second[1];
			
			
		}
	}

/*
	std::map<view_index, Eigen_vec2> views_final_center;
	for(const auto& kv : views_centers) {
		view_index idx = kv.first;
		const auto& centers = kv.second;
		
		Eigen_vec2 mean(0.0, 0.0);
		for(Eigen_vec2 center : centers) mean += center;
		mean /= centers.size();
		
		views_final_center[idx] = centers[0];
		idx.x += 10000; views_final_center[idx] = centers[1];
		idx.x += 10000; views_final_center[idx] = centers[2];
		idx.x += 10000; views_final_center[idx] = centers[3];
	}
	
	
	
	std::vector<camera> cameras;
	for(const auto& kv : views_final_center) {
		view_index idx = kv.first;
		Eigen_vec2 t = kv.second;
		
		camera cam;
		cam.name = "cam";//datas.view(idx).camera_name();
		cam.intrinsic = K;
		cam.extrinsic <<
			1, 0, 0, t[0],
			0, 1, 0, t[1],
			0, 0, 1, 0,
			0, 0, 0, 1;
		cameras.push_back(cam);
	}
	
	
	write_cameras_file(out_cams_filename, cameras);
*/
}
#endif
