#include "../lib/args.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "lib/image_correspondence.h"
#include <algorithm>

using namespace tlz;

const bool verbose = true;
const real lattice_avg_percentile = 0.8;

vec2 avg_lattice_vector(std::vector<vec2>& samples, bool horiz) {
	auto slope = [horiz](const vec2& vec) {
		if(horiz) return vec[1]/vec[0];
		else return vec[0]/vec[1];
	};
	std::sort(samples.begin(), samples.end(),
		[&slope](const vec2& a, const vec2& b) { return slope(a) < slope(b); });

	std::ptrdiff_t border = samples.size() * (1.0 - lattice_avg_percentile)/2.0;
	vec2 mean(0.0, 0.0);
	for(auto it = samples.begin()+border; it != samples.end()-border; ++it) mean += *it;
	mean *= real(1.0 / (samples.size()-2*border));
	
	return mean;
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json out_cors.json expected_x_count expected_y_count [use_depth?]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	int expected_x_count = int_arg();
	int expected_y_count = int_arg();
	bool use_depth = bool_opt_arg("use_depth", false);
	
	dataset_group datag = datas.group(cors.dataset_group);
	
	const int min_horizontal_count = expected_x_count * 0.7;
	const int min_vertical_count = expected_y_count * 1.0;
	const real min_completion = 0.5;
	const real max_depth_diff = 70.0;
	const real max_lattice_deviation = 2.0;
	const real max_bad_points = 0.1;
	
	auto filter_feature = [&](image_correspondence_feature& feature) -> bool {
		auto have = [&feature](int x, int y) -> bool {
			auto it = feature.points.find(view_index(x, y));
			return (it != feature.points.end());
		};
		auto pos = [&feature](int x, int y) -> vec2 {
			return feature.points.at(view_index(x, y)).position;
		};
		auto depth = [&feature](int x, int y) -> real {
			return feature.points.at(view_index(x, y)).depth;
		};
		
		
		Assert(have(feature.reference_view.x, feature.reference_view.y));


		// count existing views on middle horizontal axis (x_i, y_mid)
		// must be >= min_horizontal_count
		const int y_mid = feature.reference_view.y;
		int have_horizontal_count = 0;
		for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step())
			if(have(x, y_mid)) have_horizontal_count++;
						
		if(verbose) std::cout << "have horizontal: " << have_horizontal_count << ", need " << min_horizontal_count << std::endl;
		if(have_horizontal_count < min_horizontal_count) return false;
		
		// count existing views on middle vertical axis (x_mid, y_i)
		// must be >= min_vertical_count
		const int x_mid = feature.reference_view.x;
		int have_vertical_count = 0;
		for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step())
			if(have(x_mid, y)) have_vertical_count++;

		if(verbose) std::cout << "have vertical: " << have_vertical_count << ", need " << min_vertical_count << std::endl;
		if(have_vertical_count < min_vertical_count) return false;
		
		// count existing views
		// must be >= min_completion * total number of indices
		const int all_count = expected_x_count * expected_y_count;
		const int min_all_count = min_completion * all_count;
		int have_all_count = 0;
		for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step())
		for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step())
			if(have(x, y)) have_all_count++;
			
		if(verbose) std::cout << "have total: " << have_all_count << ", need " << min_all_count << std::endl;
		if(have_all_count < min_all_count) return false;

		
		// estimate (u, v) lattice vectors
		vec2 lattice_u, lattice_v;
		{
			std::vector<vec2> lattice_u_vectors;
			for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
				for(int x = datas.x_min() + datas.x_step(); x <= datas.x_max(); x += datas.x_step()) {
					int x_prev = x - datas.x_step();
					if(have(x_prev, y) && have(x, y)) lattice_u_vectors.push_back(pos(x, y) - pos(x_prev, y));
				}
			}
			lattice_u = avg_lattice_vector(lattice_u_vectors, true);
			
			std::vector<vec2> lattice_v_vectors;
			for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step()) {
				for(int y = datas.y_min() + datas.y_step(); y <= datas.y_max(); y += datas.y_step()) {
					int y_prev = y - datas.y_step();
					if(have(x, y_prev) && have(x, y)) lattice_v_vectors.push_back(pos(x, y) - pos(x, y_prev));
				}
			}
			lattice_v = avg_lattice_vector(lattice_v_vectors, false);
		}
		if(verbose) std::cout << "estimated lattice u=" << lattice_u << ", v=" << lattice_v << std::endl;
		

		// remove points that deviate too far from lattice
		std::vector<view_index> bad_points;
		vec2 ref_pos = pos(x_mid, y_mid);
		real max_sq_dev = sq(max_lattice_deviation);
		for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step()) {
			for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
				if(! have(x, y)) continue;
				vec2 have_pos = pos(x, y);
				vec2 lat_pos = ref_pos + lattice_u*(x - x_mid) + lattice_v*(y - y_mid);
				vec2 dev = lat_pos - have_pos;
				real sq_dev = sq(dev[0]) + sq(dev[1]);
				if(sq_dev > max_sq_dev) bad_points.push_back(view_index(x, y));
			}
		}
		int max_bad_points_count = max_bad_points*have_all_count;
		if(bad_points.size() > max_bad_points_count) {
			if(verbose) std::cout << "bad points (not on lattice): " << bad_points.size() << " > " << max_bad_points_count << "; rejected feature" << std::endl;
			return false;
		} else if(bad_points.size() >= 1) {
			if(verbose) std::cout << "bad points (not on lattice): " << bad_points.size() << " > " << max_bad_points_count << "; removing them" << std::endl;
			for(const view_index& idx : bad_points) feature.points.erase(idx);
		}

		
		if(use_depth) {
			real min_d = +INFINITY, max_d = 0.0;
			for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step())
			for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
				if(! have(x, y)) continue;
				real d = depth(x, y);
				if(d == 0.0 || std::isnan(d)) continue;
				if(d < min_d) min_d = d;
				if(d > max_d) max_d = d;
			}
			if(max_d - min_d > max_depth_diff) {
				if(verbose) std::cout << "depth diff: " << (max_d - min_d) << " > " << max_depth_diff << std::endl;
				return false;
			}
			std::cout << "depth differences ok" << std::endl;
		}
	
		std::cout << "accepted" << std::endl;
		return true;
	};
	
	image_correspondences out_cors;
	out_cors.dataset_group = cors.dataset_group;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		
		if(verbose) std::cout << feature_name << ":" << std::endl;
		
		image_correspondence_feature feature = kv.second;
		bool keep = filter_feature(feature);
		if(keep) out_cors.features[feature_name] = feature;
		
		if(verbose) std::cout << std::endl;
		else std::cout << '.' << std::flush;
	}
	std::cout << std::endl;
	
	std::cout << "keeping " << out_cors.features.size() << " of " << cors.features.size() << " features" << std::endl;
	
	export_image_corresponcences(out_cors, out_cors_filename);
}
