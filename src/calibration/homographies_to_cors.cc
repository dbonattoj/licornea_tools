#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "lib/image_correspondence.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "src_cors.json homographies.json out_dst_cors.json");
	image_correspondences src_cors = image_correspondences_arg();
	json j_homographies = json_arg();
	std::string out_dst_cors_filename = out_filename_arg();
	
	std::map<view_index, mat33> homographies;
	for(auto it = j_homographies.begin(); it != j_homographies.end(); ++it)
		homographies[decode_view_index(it.key())] = decode_mat(it.value());
	
	
	image_correspondences dst_cors;
	dst_cors.reference = src_cors.reference;
	
	for(const auto& kv : src_cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& src_feature = kv.second;

		image_correspondence_feature& dst_feature = dst_cors.features[feature_name];
		
		for(const auto& kv2 : src_feature.points) {
			const view_index& idx = kv2.first;
			auto hom_it = homographies.find(idx);
			if(hom_it == homographies.end()) continue;
			
			const mat33& homography = hom_it->second;
			const vec2& src_pt = kv2.second;
			
			vec2 dst_pt = mul_h(homography, src_pt);
			dst_feature.points[idx] = dst_pt;
		}
	}
	
	export_json_file(encode_image_correspondences(dst_cors), out_dst_cors_filename);
}
