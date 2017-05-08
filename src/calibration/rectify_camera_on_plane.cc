int main() {}
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
	std::cout << "usage: rectify_camera_on_plane dataset_parameters.json image_correspondences.json intrinsics.json out_cams.json out_homographies.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_cams_filename = argv[4];
	std::string out_homographies_filename = argv[5];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);

	std::cout << "loading intrinsic matrix" << std::endl;
	const json& j_intrinsics = import_json_file(intrinsics_filename);
	cv::Mat3d K = decode_mat_cv(j_intrinsics["K"]);
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	view_index reference_idx = cors.reference;

	real x_step = 1.0, y_step = 1.0;
	
	
	std::cout << "defining rectified feature points and calculating homographies" << std::endl;
	for(int y : datas.y_indices()) for(int x : datas.x_indices()) {		
		view_index idx(x, y);
		view_point_correspondence point_cor = view_points_cor[idx];

		int off_x = idx.x - reference_idx.x, off_y = idx.y - reference_idx.y;

		std::vector<cv::Point2> source_points;
		std::vector<cv::Point2> destination_points;

		for(const auto& kv : cors.features) {
			const std::string& feature_name = kv.first;
			const image_correspondence_feature& feature = kv.second;

			real ix_step = 2.0;
			real iy_step = 2.0; // TODO...

			Eigen_vec2 reference_point = feature.points.at(reference_idx);
			Eigen_vec2 source_feature_point = feature.points(idx);
			Eigen_vec2 destination_feature_point(reference_point + ix_step * off_x, reference_point + iy_step * off_y);

			source_points.emplace_back(source_feature_point[0], source_feature_point[1]);
			destination_points.emplace_back(destination_feature_point[0], destination_feature_point[1]);

			point_cor.source_points.emplace_back();
		}

		Eigen_vec2 camera_center_position = Eigen_vec2(x_step * off_x, y_step * off_y, 0.0);

		cv::Mat homography = cv::findHomography(source_points, destination_points, 0);
		
		

		std::cout << '.' << std::flush;
	}

	std::cout << "done" << std::endl;
}
#endif
