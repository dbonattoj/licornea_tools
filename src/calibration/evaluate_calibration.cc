#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/misc.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <iostream>
#include <cmath>
#include <random>
#include <fstream>

using namespace tlz;

constexpr bool verbose = true;
constexpr int min_features_count = 10;
constexpr real max_reprojection_error = 100.0;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json cams.json out_samples.txt [random/all] [random_count=100000]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	camera_array cams = cameras_arg();
	std::string out_samples_filename = out_filename_arg();
	std::string mode = enum_opt_arg({"random", "all"}, "all");
	int random_count = int_opt_arg(10000);
	
	auto cams_map = cameras_map(cams);
	
	std::ofstream out_samples_stream(out_samples_filename);
	out_samples_stream << "baseline reprojection_error\n";


	auto warp = [&](const view_index& from, const view_index& to) -> real {
		if(cams_map.find(datas.view(from).camera_name()) == cams_map.end()) return NAN;
		if(cams_map.find(datas.view(to).camera_name()) == cams_map.end()) return NAN;
		
		const camera& from_cam = cams_map.at(datas.view(from).camera_name());
		const camera& to_cam = cams_map.at(datas.view(to).camera_name());
		intrinsics from_intr = to_undistorted_intrinsics(from_cam, datas.image_width(), datas.image_height());
		intrinsics to_intr = to_undistorted_intrinsics(to_cam, datas.image_width(), datas.image_height());
		mat44 pose_transformation = to_cam.extrinsic() * from_cam.extrinsic_inv();
		
		const feature_points& from_fpoints = undistorted_feature_points_for_view(cors, from, from_intr);
		const feature_points& to_fpoints = undistorted_feature_points_for_view(cors, to, to_intr);
		std::vector<std::string> common_features;
		for(const auto& kv : from_fpoints.points) {
			const std::string& feature_name = kv.first;
			if(to_fpoints.points.find(feature_name) != to_fpoints.points.end())
				common_features.push_back(feature_name);
		}
		if(common_features.size() == 0) return NAN;
		
		std::vector<real> reprojection_errors;
		reprojection_errors.reserve(common_features.size());
		
		for(const std::string& feature_name : common_features) {
			const feature_point& from_fpoint = from_fpoints.points.at(feature_name);
			const feature_point& to_fpoint = to_fpoints.points.at(feature_name);
				
			vec3 from_i = vec3(from_fpoint.position[0], from_fpoint.position[1], 1.0) * from_fpoint.depth;
			vec3 from_v = from_intr.K_inv * from_i;
			vec3 to_v = mul_h(pose_transformation, from_v);
			vec3 to_i = to_intr.K * to_v;
			to_i /= to_i[2];
			
			real reprojection_error = sq(to_i[0] - to_fpoint.position[0]) + sq(to_i[1] - to_fpoint.position[1]);
			reprojection_errors.push_back(reprojection_error);
		}
		if(reprojection_errors.size() < min_features_count) return NAN;
		
		std::ptrdiff_t mid = reprojection_errors.size() / 2;
		std::nth_element(reprojection_errors.begin(), reprojection_errors.begin()+mid, reprojection_errors.end());
		
		real err = reprojection_errors[mid];
		if(err > max_reprojection_error) return NAN;
		
		int baseline = std::sqrt(sq(from.x - to.x) + sq(from.y - to.y));
		//real baseline = std::sqrt(sq(pose_transformation(0,3)) + sq(pose_transformation(1,3)) + sq(pose_transformation(2,3)));
		
		#pragma omp critical
		{
			out_samples_stream << err << ' ' << baseline << '\n';
		}
		
		if(verbose)
			std::cout << from << " ->" << to << ": err" << std::endl;
		
		return err;
	};


	auto indices = datas.indices();	

	if(mode == "random") {
		std::mt19937 gen;
		std::uniform_int_distribution<std::ptrdiff_t> dist(0, indices.size() - 1);

		#pragma omp parallel for
		for(int i = 0; i < random_count; ++i) {
			const view_index& from_idx = indices[dist(gen)];
			const view_index& to_idx = indices[dist(gen)];
			warp(from_idx, to_idx);
			std::cout << '.' << std::flush;
		}


	} else if(mode == "all") {
		#pragma omp parallel for
		for(int from = 0; from < indices.size(); ++from) {
			for(int to = from + 1; to < indices.size(); ++to) {
				const view_index& from_idx = indices[from];
				const view_index& to_idx = indices[to];
				warp(from_idx, to_idx);
				std::cout << '.' << std::flush;
			}
		}
	}
}
