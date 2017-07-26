#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/misc.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include "lib/cg/references_grid.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cors.json pseudo_refgrid.json outreach_radius out_cors.json");
	image_correspondences cors = image_correspondences_arg();
	references_grid pseudo_refgrid = references_grid_arg();
	int outreach_radius = int_arg();
	std::string out_cors_filename = out_filename_arg();

	int outreach_radius_sq = sq(outreach_radius);

	auto refgrid = get_references_grid(cors);
	for(std::ptrdiff_t col = 0; col < refgrid.cols(); ++col)
	for(std::ptrdiff_t row = 0; row < refgrid.rows(); ++row) {
		view_index idx = refgrid.view(col, row);
		if(! pseudo_refgrid.has_view(idx)) {
			std::cout << "pseudo refgrid does not have view " << idx << std::endl;
			return 0;
		}
	}
	
	for(std::ptrdiff_t col = 0; col < pseudo_refgrid.cols(); ++col)
	for(std::ptrdiff_t row = 0; row < pseudo_refgrid.rows(); ++row) {
		view_index idx = pseudo_refgrid.view(col, row);
		if(feature_points_for_view(cors, idx).count() == 0) {
			std::cout << "no features for pseudo reference view " << idx << std::endl;
			return 0;
		}
	}

	auto ref_vws = get_reference_views(cors);
	
	image_correspondences new_cors;
	
	for(const view_index& ref_idx : ref_vws) {
		std::cout << "reference view " << ref_idx << std::endl;
		auto ref_cors = image_correspondences_with_reference(cors, ref_idx);

		for(std::ptrdiff_t col = 0; col < pseudo_refgrid.cols(); ++col)
		for(std::ptrdiff_t row = 0; row < pseudo_refgrid.rows(); ++row) {
			view_index pseudo_idx = pseudo_refgrid.view(col, row);
			std::cout << "   pseudo reference view " << pseudo_idx << std::endl;

			for(const auto& kv : ref_cors.features) {
				const std::string& feature_name = kv.first;
				const image_correspondence_feature& feature = kv.second;
				
				
				image_correspondence_feature new_feature;
				new_feature.reference_view = pseudo_idx;
		
				for(const auto& kv2 : feature.points) {
					const view_index& idx = kv2.first;
					const feature_point& fpoint = kv2.second;
					
					if(sq(idx.x - pseudo_idx.x) + sq(idx.y - pseudo_idx.y) <= outreach_radius_sq)
						new_feature.points[idx] = fpoint;
				}
				
				if(new_feature.points.size() > 0) {
					std::string new_feature_name = feature_name + "#p" + std::to_string(col) + "," + std::to_string(row);
					new_cors.features[new_feature_name] = new_feature;
				}
			}
		}
	}
	
	export_image_corresponcences(new_cors, out_cors_filename);
}

