#include "image_correspondence.h"
#include "../../lib/utility/string.h"
#include <iostream>

namespace tlz {


image_correspondence_feature decode_image_correspondence_feature(const json& j_feat) {
	image_correspondence_feature feat;
	
	if(j_feat.count("depth") == 1) feat.depth = j_feat["depth"];

	const json& j_pts = j_feat["points"];
	for(auto it = j_pts.begin(); it != j_pts.end(); ++it) {	
		std::string key = it.key();	
		view_index idx = decode_view_index(key);
		
		const json& j_pos = it.value();
		Eigen_vec2 pos(j_pos[0], j_pos[1]);

		feat.points[idx] = pos;
	}
	
	return feat;
}


json encode_image_correspondence_feature(const image_correspondence_feature& feat) {
	json j_pts = json::object();
	for(const auto& pt : feat.points) {
		view_index idx = pt.first;
		std::string key = encode_view_index(idx);
		
		Eigen_vec2 pos = pt.second;
		json j_pos = json::array();
		j_pos.push_back(pos[0]);
		j_pos.push_back(pos[1]);
		
		j_pts[key] = j_pos;
	}
	
	json j_feat = json::object();
	j_feat["points"] = j_pts;
	if(feat.depth != 0.0) j_feat["depth"] = feat.depth;
		
		//std::cout << j_feat["points"].size() << " point for feature" << std::endl;
		
	return j_feat;
}


image_correspondences import_image_correspondences_file(const std::string& filename) {
	image_correspondences cors;
	json j_cors = import_json_file(filename);
	if(j_cors.count("reference") == 1) cors.reference = decode_view_index(j_cors["reference"]);
	const json& j_features = j_cors["features"];
	for(auto it = j_features.begin(); it != j_features.end(); ++it) {
		std::string feature_name = it.key();
		const json& j_feature = it.value();
		cors.features[feature_name] = decode_image_correspondence_feature(j_feature);
	}
	return cors;
}


void export_image_correspondences_file(const std::string& filename, const image_correspondences& cors) {
	json j_cors = json::object();
	if(cors.reference) j_cors["reference"] = encode_view_index(cors.reference);
	json j_features = json::object();
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		j_features[feature_name] = encode_image_correspondence_feature(feature);
	}
	j_cors["features"] = j_features;
	export_json_file(j_cors, filename);
}



}
