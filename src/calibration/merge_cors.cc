#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "lib/image_correspondence.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_cors1.json in_cors2.json out_cors.json");
	image_correspondences in_cors1 = image_correspondences_arg();
	image_correspondences in_cors2 = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();

	if(in_cors1.dataset_group != in_cors2.dataset_group)
		throw std::runtime_error("input cors must have same dataset group");
	
	image_correspondences out_cors = in_cors1;
	for(const auto& kv : in_cors2.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		auto existing_it = out_cors.features.find(feature_name);
		if(existing_it == out_cors.features.end()) {
			out_cors.features[feature_name] = feature;
		} else {
			image_correspondence_feature& existing_feature = existing_it->second;
			if(existing_feature.reference_view != feature.reference_view)
				throw std::runtime_error("same name features with different reference views");
			existing_feature.points.insert(feature.points.begin(), feature.points.end());
		}
	}
	
	export_image_corresponcences(out_cors, out_cors_filename);
}

