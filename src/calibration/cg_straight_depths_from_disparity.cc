#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include "lib/cg/straight_depths.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/intrinsics.h"
#include "../lib/json.h"
#include "../lib/eigen.h"
#include "../lib/assert.h"
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseLU>
#include <Eigen/SparseQR>

using namespace tlz;

constexpr bool verbose = true;

using view_feature_positions = std::map<view_index, vec2>;

using view_feature_position_pair = std::pair<vec2, vec2>;
using view_feature_position_pairs = std::map<view_index, view_feature_position_pair>;

constexpr int min_position_pairs_count = 300;


view_feature_position_pairs common_view_feature_positions(
	const view_feature_positions& views1, const view_feature_positions& views2
) {
	view_feature_position_pairs out_pairwise;
	auto it1 = views1.begin();
	auto it2 = views2.begin();
	while(it1 != views1.end() && it2 != views2.end()) {
		if(it1->first < it2->first) {
			++it1;
		} else if(it2->first < it1->first) {
			++it2;
		} else {
			const view_index& idx = it1->first;			
			out_pairwise.emplace(idx, view_feature_position_pair(it1->second, it2->second));
			++it1;
			++it2;
		} 
	}
	return out_pairwise;
}



struct estimate_relative_scale_result {
	real scale = NAN;
	real confidence = 0.0;
};
estimate_relative_scale_result estimate_relative_scale(const view_feature_position_pairs& ref_tg_positions) {
	// find scale s and translation t which maps reference positions ref_i to target positions tg_i:
	// tg_i = s * (ref_i + t)
	
	// solve using linear least squares system Ax = b with x = [s, s*tx, s*ty]
	
	std::size_t n = ref_tg_positions.size();

	// fill matrices A, b
	Eigen_matXn<3> A(2*n, 3);
	Eigen_vecX b(2*n);
	std::ptrdiff_t row = 0;
	for(const auto& kv : ref_tg_positions) {
		const view_feature_position_pair& p = kv.second;
		const vec2& ref_position = p.first;
		const vec2& tg_position = p.second;
		
		A(row, 0) = ref_position[0];
		A(row, 1) = 1.0;
		A(row, 2) = 0.0;
		b[row] = tg_position[0];
		row++;

		A(row, 0) = ref_position[1];
		A(row, 1) = 0.0;
		A(row, 2) = 1.0;
		b[row] = tg_position[1];
		row++;
	}
	
	// solve using linear least squares
	Eigen::ColPivHouseholderQR<Eigen_matXn<3>> solver(A);
	if(solver.info() != Eigen::Success) return estimate_relative_scale_result();
	Eigen_vec<3> x = solver.solve(b);
	if(solver.info() != Eigen::Success) return estimate_relative_scale_result();
	
	//std::cout << std::setprecision(10) << std::endl;
		
	// measure error of solution
	real error = 0.0;
	for(const auto& kv : ref_tg_positions) {
		const view_feature_position_pair& p = kv.second;
		const vec2& ref_position = p.first;
		const vec2& tg_position = p.second;
	
		vec2 ref_to_tg_position(
			x[0]*ref_position[0] + x[1],
			x[0]*ref_position[1] + x[2]
		);

		vec2 diff = tg_position - ref_to_tg_position;
		error += sq(diff[0]) + sq(diff[1]);
	}
	real confidence = real(n) / error;
	
	return {x[0], confidence};
}



Eigen_vecX complete_straight_depths(const Eigen_vecX& known_depths, const Eigen_matXX& ratios, const Eigen_matXX& weights) {
	std::size_t ratios_count = 0;
	std::size_t known_count = 0;
	std::size_t features_count = ratios.rows();
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg)
		if(weights(tg, ref) != 0.0) ++ratios_count;
	for(int known = 0; known < features_count; ++known)
		if(known_depths[known] != 0.0) ++known_count;
	
	std::size_t rows = ratios_count + known_count;
	std::size_t cols = features_count;
	
	Eigen::SparseMatrix<real> A(rows, cols);
	Eigen::SparseVector<real> b(rows);
	std::ptrdiff_t row = 0;
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg) {
		real weight = weights(tg, ref);
		if(weight == 0.0) continue;
		
		A.insert(row, ref) = ratios(tg, ref);
		A.insert(row, tg) = -1.0;
		
		++row;
	}
	
	for(int known = 0; known < features_count; ++known) {
		if(known_depths[known] == 0.0) continue;
		
		b.insert(row) = known_depths[known];
		A.insert(row, known) = 1.0;

		++row;
		
		std::cout << "known: " << known << " := " << known_depths[known] << std::endl;
	}
	
	A.makeCompressed();
	
	Eigen::SparseQR<Eigen::SparseMatrix<real>, Eigen::COLAMDOrdering<int>> solver;
	
	solver.compute(A);
	//if(solver.info() != Eigen::Success) throw std::runtime_error("solver.compute(A) failed");
	
	Eigen_vecX x = solver.solve(b);
	//if(solver.info() != Eigen::Success) throw std::runtime_error("solver.solve(b) failed");

	Assert(x.rows() == cols);

	real rms_error = 0.0;
	for(int known = 0; known < features_count; ++known) {
		if(known_depths[known] == 0.0) continue;
		real in = known_depths[known];
		real out = x[known];
		rms_error += sq(in - out);
	}
	rms_error = std::sqrt(rms_error / known_count);
	std::cout << "rms_error=" << rms_error << std::endl;

	return x;
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image_correspondences.json intrinsics.json R.json straight_depths.json out_straight_depths.json");
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	straight_depths in_depths = straight_depths_arg();
	std::string out_depths_filename = out_filename_arg();

	Eigen::initParallel();
	
	const int default_num_threads = Eigen::nbThreads();
	Eigen::setNbThreads(0);

	mat33 unrotate = intr.K * R.t() * intr.K_inv;

	auto all_views = get_all_views(cors);
	auto all_features = get_feature_names(cors);

	//auto all_features2 = all_features;
	//all_features.clear();
	//for(int i = 0; i < 100; ++i) all_features.push_back(all_features2[i]);


	std::map<std::string, int> feature_indices;
	for(std::ptrdiff_t i = 0; i < all_features.size(); ++i)
		feature_indices[all_features[i]] = i;


	std::size_t features_count = all_features.size();
	
	
	// undistort & unrotate correspondences, and get view feature positions foreach feature
	std::cout << "undistorting correspondences" << std::endl;
	image_correspondences undist_cors = undistort(cors, intr);	
	std::cout << "collecting unrotated view feature positions" << std::endl;
	std::map<std::string, view_feature_positions> all_view_feature_positions;
	for(const auto& kv : undist_cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		view_feature_positions positions;
		for(const auto& kv2 : feature.points) {
			const view_index& idx = kv2.first;
			const feature_point& fpoint = kv2.second;
			vec2 unrotated_position = mul_h(unrotate, fpoint.position);
			all_view_feature_positions[feature_name][idx] = unrotated_position;
		}
	}

	// calculate relative depth rations for all feature pairs, using the image correspondences	
	Eigen_matXX depth_ratios(features_count, features_count);
	Eigen_matXX weights(features_count, features_count);
	weights.setZero();

	std::cout << "feature count: " << features_count << std::endl;

	#pragma omp parallel for schedule(guided)
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg) {
		// get feature position pairs for reference and target feature
		// for views on which both features are present
		const std::string& ref_feature_name = all_features.at(ref);
		const view_feature_positions& ref_positions = all_view_feature_positions.at(ref_feature_name);
		const std::string& tg_feature_name = all_features.at(tg);
		const view_feature_positions& tg_positions = all_view_feature_positions.at(tg_feature_name);
			
		auto ref_tg_position_pairs = common_view_feature_positions(ref_positions, tg_positions);
		if(ref_tg_position_pairs.size() < min_position_pairs_count) continue;
		
		// estimate scale of target view feature positions, relative to corresponding reference view features
		estimate_relative_scale_result result = estimate_relative_scale(ref_tg_position_pairs);
		if(std::isnan(result.scale)) continue;
		
		//#pragma omp critical
		//std::cout << "ref=" << ref << ", tg=" << tg << ", scale=" << scale << ", inv_scale=" << 1.0/scale << std::endl;

		depth_ratios(tg, ref) = 1.0 / result.scale;
		weights(tg, ref) = result.confidence;

		if(tg == features_count-1) std::cout << '.' << std::flush;
	}
	std::cout << std::endl;

	{
		cv::Mat_<real> weights_img;
		cv::eigen2cv(weights, weights_img);
		cv::Mat_<uchar> weights_img2;
		cv::normalize(weights_img, weights_img2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::resize(weights_img2, weights_img2, cv::Size(0,0), 3, 3, cv::INTER_NEAREST);
		cv::imwrite("w.png", weights_img2);

		cv::Mat_<real> ratios_img;
		cv::eigen2cv(depth_ratios, ratios_img);
		cv::Mat_<uchar> ratios_img2;
		cv::normalize(ratios_img, ratios_img2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::resize(ratios_img2, ratios_img2, cv::Size(0,0), 3, 3, cv::INTER_NEAREST);
		cv::imwrite("r.png", ratios_img2);
	}
	
	
	// deduce depths of all features
	std::cout << "computing depths for all features, from pairwise ratios and known depths" << std::endl;
	Eigen_vecX known_depths(features_count);
	known_depths.setZero();
	for(const auto& kv : in_depths) {
		const std::string& feature_name = kv.first;
		const straight_depth& sdepth = kv.second;
		try {
		int feature_index = feature_indices.at(feature_name);
		known_depths[feature_index] = sdepth.depth;
		} catch(...) {}
	}

	Eigen::setNbThreads(default_num_threads);
	
	Eigen_vecX completed_depths = complete_straight_depths(known_depths, depth_ratios, weights);
	
	Eigen_matXn<2> known_completed(features_count, 2);
	known_completed.setZero();
	known_completed.block(0, 0, features_count, 1) = known_depths;
	known_completed.block(0, 1, features_count, 1) = completed_depths;
	
	std::cout << std::setprecision(10);
	//std::cout << "known/completed:\n" << known_completed << std::endl;

	std::cout << "saving computed straight depths" << std::endl;
	straight_depths out_straight_depths;
	for(int known = 0; known < features_count; ++known) {
		real depth = completed_depths[known];
		out_straight_depths[all_features[known]] = depth;
	}
	export_json_file(encode_straight_depths(out_straight_depths), out_depths_filename);
}
