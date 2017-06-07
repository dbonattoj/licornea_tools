#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/view_homography.h"
#include "lib/image_correspondence.h"
#include "lib/feature_point.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "src_cors.json homographies.json out_dst_cors.json");
	image_correspondences src_cors = image_correspondences_arg();
	view_homographies homographies = homographies_arg();
	std::string out_dst_cors_filename = out_filename_arg();	
	
	image_correspondences dst_cors;
	
	for(const auto& kv : src_cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& src_feature = kv.second;

		image_correspondence_feature& dst_feature = dst_cors.features[feature_name];
		// dst_feature has no reference_view
		
		for(const auto& kv2 : src_feature.points) {
			const view_index& idx = kv2.first;
			auto hom_it = homographies.find(idx);
			if(hom_it == homographies.end()) continue;
			
			const mat33& H = hom_it->second.mat;
			const feature_point& src_fpt = kv2.second;
			feature_point& dst_fpt = dst_feature.points[idx];
			
			dst_fpt.position = mul_h(H, src_fpt.position);
			dst_fpt.depth = src_fpt.depth;
			dst_fpt.weight = src_fpt.weight;
		}
	}
	
	export_json_file(encode_image_correspondences(dst_cors), out_dst_cors_filename);
}
