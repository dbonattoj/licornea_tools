#include "../lib/args.h"
#include "../lib/dataset.h"
#include "lib/image_correspondence.h"

using namespace tlz;

const bool verbose = true;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json out_cors.json [expected_x_count] [expected_y_count]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	int expected_x_count = int_opt_arg(300);
	int expected_y_count = int_opt_arg(22);
	
	dataset_group datag = datas.group(cors.dataset_group);
	
	const int min_horizontal_count = expected_x_count * 0.7;
	const int min_vertical_count = expected_y_count * 1.0;
	const real min_completion = 0.5;
	const real max_horizontal_y_diff = 10.0;
	const real max_vertical_x_diff = 10.0;
	
	auto filter_feature = [&](image_correspondence_feature& feature) -> bool {
		auto have = [&feature](int x, int y) -> bool {
			auto it = feature.points.find(view_index(x, y));
			return (it != feature.points.end());
		};
		auto pos = [&feature](int x, int y) -> vec2 {
			return feature.points.at(view_index(x, y)).position;
		};


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

			
		// maximal Y deviation of feature positions for views on all horizontal axes
		// must be <= max_horizontal_y_diff
		for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
			real min_iy = +INFINITY, max_iy = -INFINITY;
			for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step()) {
				if(! have(x, y)) continue;
				real iy = pos(x, y)[1];
				if(iy < min_iy) min_iy = iy;
				if(iy > max_iy) max_iy = iy;
			}
			if(max_iy - min_iy > max_horizontal_y_diff) {
				if(verbose) std::cout << "row y=" << y << " iy diff: " << (max_iy - min_iy) << " > " << max_horizontal_y_diff << std::endl;
				return false;
			}
		}
		
		// maximal X deviation of feature positions for views on all vertical axes
		// must be <= max_vertical_x_diff
		for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step()) {
			real min_ix = +INFINITY, max_ix = -INFINITY;
			for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
				if(! have(x, y)) continue;
				real ix = pos(x, y)[0];
				if(ix < min_ix) min_ix = ix;
				if(ix > max_ix) max_ix = ix;
			}
			if(max_ix - min_ix > max_vertical_x_diff) {
				if(verbose) std::cout << "col x=" << x << " ix diff: " << (max_ix - min_ix) << " > " << max_vertical_x_diff << std::endl;
				return false;
			}
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
	
	export_json_file(encode_image_correspondences(out_cors), out_cors_filename);
}
