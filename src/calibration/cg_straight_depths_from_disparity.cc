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

constexpr bool verbose = false;

using view_feature_positions = std::map<view_index, vec2>;

using view_feature_position_pair = std::pair<vec2, vec2>;
using view_feature_position_pairs = std::map<view_index, view_feature_position_pair>;

constexpr int min_position_pairs_count = 300;
constexpr real max_relative_scale_error = 0.3;

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
	real weight = 0.0;
	
	explicit operator bool () const { return ! std::isnan(scale); }
};
estimate_relative_scale_result estimate_relative_scale(
	const view_feature_position_pairs& src_tg_positions,
	const view_index& center_view = view_index()
) {
	// find scale s and translation t which maps source positions src_i to target positions tg_i:
	// tg_i = s*src_i + t

	real scale;
	vec2 translation;
	
	if(center_view) {
		// center on center_view, and solve for scale only
		vec2 src_center_position = src_tg_positions.at(center_view).first;
		vec2 tg_center_position = src_tg_positions.at(center_view).second;
		
		
		real scales_sum = 0, scales_weights_sum = 0;
		for(const auto& kv : src_tg_positions) {
			const view_index& idx = kv.first;
			if(idx == center_view) continue;
			const view_feature_position_pair& p = kv.second;
			const vec2& src_position = p.first;
			const vec2& tg_position = p.second;
			
			const vec2& src_offset = src_position - src_center_position;
			const vec2& tg_offset = tg_position - tg_center_position;
						
			real x_scale = tg_offset[0] / src_offset[0];
			real x_weight = std::max({ sq(src_offset[0]), sq(tg_offset[0]) });
			scales_sum += x_weight * x_scale;
			scales_weights_sum += x_weight;
			
			real y_scale = tg_offset[1] / src_offset[1];
			real y_weight = std::max({ sq(src_offset[1]), sq(tg_offset[1]) });
			scales_sum += y_weight * y_scale;
			scales_weights_sum += y_weight;
		}
		
		scale = scales_sum / scales_weights_sum;
		translation = tg_center_position - src_center_position*scale;

	} else {
		// solve using linear least squares system Ax = b with x = [s, tx, ty]
		std::size_t n = src_tg_positions.size();
	
		// fill matrices A, b
		Eigen_matXn<3> A(2*n, 3);
		Eigen_vecX b(2*n);
		std::ptrdiff_t row = 0;
		for(const auto& kv : src_tg_positions) {
			const view_feature_position_pair& p = kv.second;
			const vec2& src_position = p.first;
			const vec2& tg_position = p.second;
			
			A(row, 0) = src_position[0];
			A(row, 1) = 1.0;
			A(row, 2) = 0.0;
			b[row] = tg_position[0];
			row++;
	
			A(row, 0) = src_position[1];
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

		scale = x[0];
		translation = vec2(x[1], x[2]);
	}
	
	
	
	// measure error of solution
	real error = 0.0;
	for(const auto& kv : src_tg_positions) {
		const view_feature_position_pair& p = kv.second;
		const vec2& src_position = p.first;
		const vec2& tg_position = p.second;

		vec2 src_to_tg_position = scale*src_position + translation;

		vec2 diff = tg_position - src_to_tg_position;
		error += sq(diff[0]) + sq(diff[1]);
	}
	error = std::sqrt(error / real(src_tg_positions.size()));
	
	//std::cout << "err=" << error << std::endl;
	
	if(error > max_relative_scale_error) return estimate_relative_scale_result();
	
	real weight = (center_view ? 10.0 : 1.0) * 1.0/sq(error);
	
	return estimate_relative_scale_result {scale, weight};
}


Eigen_vecX compute_global_ratios(const Eigen_matXX& ratios, const Eigen_matXX& weights) {
	std::size_t ratios_count = 0;
	std::size_t features_count = ratios.rows();
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg)
		if(weights(tg, ref) != 0.0) ++ratios_count;
		
	std::size_t rows = ratios_count + 1;
	std::size_t cols = features_count;
	
	Eigen::SparseMatrix<real> A(rows, cols);
	Eigen::SparseVector<real> b(rows);
	std::ptrdiff_t row = 0;
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg) {
		real weight = weights(tg, ref);
		if(weight == 0.0) continue;
		
		A.insert(row, tg) = ratios(tg, ref);
		A.insert(row, ref) = -1.0;
				
		++row;
	}
	
	b.insert(row) = 1.0;
	A.insert(row, 0) = 1.0;	
	
	A.makeCompressed();
	
	Eigen::SparseQR<Eigen::SparseMatrix<real>, Eigen::COLAMDOrdering<int>> solver;
	
	solver.compute(A);
	//if(solver.info() != Eigen::Success) throw std::runtime_error("solver.compute(A) failed");
	
	Eigen_vecX x = solver.solve(b);
	//if(solver.info() != Eigen::Success) throw std::runtime_error("solver.solve(b) failed");

	Assert(x.rows() == cols);

	return x;
}



Eigen_vecX complete_depths(const Eigen_vecX& known_depths, const Eigen_vecX& global_ratios) {
	real ratios_sum = 0.0, known_sum = 0.0;
	
	for(std::ptrdiff_t i = 0; i < known_depths.rows(); ++i) {
		if(known_depths[i] == 0) continue;
		if(global_ratios[i] == 0) continue;
		
		ratios_sum += global_ratios[i];
		known_sum += known_depths[i];
	}
	real scale = known_sum / ratios_sum;
		
	Eigen_vecX depths = scale * global_ratios;

	real rms_error = 0.0;
	real count = 0.0;
	for(std::ptrdiff_t i = 0; i < known_depths.rows(); ++i) {
		if(known_depths[i] == 0) continue;
		if(depths[i] == 0) continue;
	
		real known = known_depths[i];
		real re_known = depths[i];
		
		rms_error += sq(known - re_known);
		count += 1.0;
	}
	rms_error = std::sqrt(rms_error / count);
	std::cout << "rms error: " << rms_error << std::endl;

	return depths;
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image_correspondences.json intrinsics.json R.json straight_depths.json out_straight_depths.json");
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	straight_depths in_depths = straight_depths_arg();
	std::string out_depths_filename = out_filename_arg();

	std::cout << std::setprecision(10);
	Eigen::initParallel();
	
	const int default_num_threads = Eigen::nbThreads();
	Eigen::setNbThreads(0);

	mat33 unrotate = intr.K * R.t() * intr.K_inv;

	auto all_views = get_all_views(cors);
	auto all_features = get_feature_names(cors);

	std::size_t features_count = all_features.size();
	//features_count = 20;
	std::cout << "feature count: " << features_count << std::endl;
	
	std::map<std::string, int> feature_indices;
	for(std::ptrdiff_t i = 0; i < features_count; ++i)
		feature_indices[all_features[i]] = i;

	// undistort & unrotate correspondences, and get view feature positions foreach feature
	// all_view_feature_xy = points in normalized view spaces (v_z = 1)
	std::cout << "undistorting correspondences" << std::endl;
	image_correspondences undist_cors = undistort(cors, intr);	
	std::cout << "collecting unrotated view feature positions" << std::endl;
	std::map<std::string, view_feature_positions> all_view_feature_xy;
	for(const auto& kv : undist_cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		view_feature_positions positions;
		for(const auto& kv2 : feature.points) {
			const view_index& idx = kv2.first;
			const feature_point& fpoint = kv2.second;
			vec2 unrotated_xy = mul_h(unrotate, fpoint.position);
			all_view_feature_xy[feature_name][idx] = unrotated_xy;
		}
	}

	// calculate relative scale ratio for all feature pairs, using the image correspondences	
	Eigen_matXX scale_ratios(features_count, features_count);
	Eigen_matXX weights(features_count, features_count);
	scale_ratios.setZero();
	weights.setZero();


	std::cout << "estimating pairwise relative scales of disparities" << std::endl;
	#pragma omp parallel for schedule(guided)
	for(int ref = 0; ref < features_count; ++ref) for(int tg = ref+1; tg < features_count; ++tg) {
		// get feature position pairs for source and target feature
		// for views on which both features are present
		const std::string& src_feature_name = all_features.at(ref);
		const view_feature_positions& src_positions = all_view_feature_xy.at(src_feature_name);
		const std::string& tg_feature_name = all_features.at(tg);
		const view_feature_positions& tg_positions = all_view_feature_xy.at(tg_feature_name);
		
		auto src_tg_position_pairs = common_view_feature_positions(src_positions, tg_positions);
		if(src_tg_position_pairs.size() < min_position_pairs_count) continue;
		
		
		// estimate scale of target view feature positions, relative to corresponding reference view features
		estimate_relative_scale_result result;
		view_index src_reference = cors.features.at(src_feature_name).reference_view;
		view_index tg_reference = cors.features.at(tg_feature_name).reference_view;
		if(src_reference == tg_reference) result = estimate_relative_scale(src_tg_position_pairs, src_reference);
		else result = estimate_relative_scale(src_tg_position_pairs);
		if(! result) continue;

		scale_ratios(tg, ref) = result.scale;
		weights(tg, ref) = result.weight;		

		if(verbose)
			std::cout << ref << "/" << (features_count-1) << " <--> " << tg << "/" << (features_count-1) << std::endl;
		else
			std::cout << '.' << std::flush;
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
		cv::eigen2cv(scale_ratios, ratios_img);
		cv::Mat_<uchar> ratios_img2;
		cv::normalize(ratios_img, ratios_img2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::resize(ratios_img2, ratios_img2, cv::Size(0,0), 5, 5, cv::INTER_NEAREST);
		cv::imwrite("r.png", ratios_img2, { CV_IMWRITE_PNG_COMPRESSION, 9 });
	}
		
	
	std::cout << "calculating global scale ratios" << std::endl;
	Eigen::setNbThreads(default_num_threads);
	Eigen_vecX global_scale_ratios = compute_global_ratios(scale_ratios, weights);
		
	
	std::cout << "calculating depths" << std::endl;
	Eigen_vecX known_depths(features_count);
	known_depths.setZero();
	for(const auto& kv : in_depths) {
		const std::string& feature_name = kv.first;
		const straight_depth& sdepth = kv.second;
		auto feature_index_it = feature_indices.find(feature_name);
		if(feature_index_it != feature_indices.end()) {
			int feature_index = feature_index_it->second;
			known_depths[feature_index] = sdepth.depth;	
		}
	}
	Eigen_vecX depths = complete_depths(known_depths, global_scale_ratios);


	Eigen_matXn<2> known_completed(features_count, 2);
	known_completed.setZero();
	known_completed.block(0, 0, features_count, 1) = known_depths;
	known_completed.block(0, 1, features_count, 1) = depths;
	
	std::cout << std::setprecision(10);
	std::cout << "known/completed:\n" << std::endl;
	for(int row = 0; row < features_count; ++row) {
		if(known_depths[row] == 0) continue;
		if(depths[row] == 0) continue;
		std::cout << known_completed(row, 0) << ";" << known_completed(row, 1) << "\n";
	}


	std::cout << "saving computed depths" << std::endl;
	straight_depths out_straight_depths;
	for(int row = 0; row < features_count; ++row) {
		if(depths[row] == 0) continue;
		real depth = depths[row];
		out_straight_depths[all_features[row]] = depth;
	}
	export_json_file(encode_straight_depths(out_straight_depths), out_depths_filename);
}
