#include "image_correspondence.h"
#include "../../lib/string.h"
#include <iostream>

namespace tlz {


image_correspondence_feature decode_image_correspondence_feature(const json& j_feat) {
	image_correspondence_feature feat;
	
	const json& j_pts = j_feat["points"];
	for(auto it = j_pts.begin(); it != j_pts.end(); ++it) {	
		std::string key = it.key();	
		view_index idx = decode_view_index(key);
			
		const json& j_pt = it.value();
		feat.points[idx] = vec2(j_pt["x"], j_pt["y"]);
		if(has(j_pt, "depth"))
			feat.point_depths[idx] = j_pt["depth"];
	}
	
	return feat;
}


json encode_image_correspondence_feature(const image_correspondence_feature& feat) {
	json j_pts = json::object();
	for(const auto& pt : feat.points) {
		view_index idx = pt.first;
		std::string key = encode_view_index(idx);
		vec2 pos = pt.second;
				
		json j_pt = json::object();
		j_pt["x"] = pos[0];
		j_pt["y"] = pos[1];
		
		auto it = feat.point_depths.find(idx); 
		if(it != feat.point_depths.end())
			j_pt["depth"] = it->second;
		
		j_pts[key] = j_pt;
	}
	
	json j_feat = json::object();
	j_feat["points"] = j_pts;
				
	return j_feat;
}


image_correspondences decode_image_correspondences(const json& j_cors) {
	image_correspondences cors;
	if(has(j_cors, "reference")) cors.reference = decode_view_index(j_cors["reference"]);
	if(has(j_cors, "dataset_group")) cors.dataset_group = j_cors["dataset_group"];
	const json& j_features = j_cors["features"];
	for(auto it = j_features.begin(); it != j_features.end(); ++it) {
		std::string feature_name = it.key();
		const json& j_feature = it.value();
		cors.features[feature_name] = decode_image_correspondence_feature(j_feature);
	}
	return cors;
}


json encode_image_correspondences(const image_correspondences& cors) {
	json j_cors = json::object();
	if(cors.reference) j_cors["reference"] = encode_view_index(cors.reference);
	if(! cors.dataset_group.empty()) j_cors["dataset_group"] = cors.dataset_group;
	json j_features = json::object();
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		j_features[feature_name] = encode_image_correspondence_feature(feature);
	}
	j_cors["features"] = j_features;
	return j_cors;
}



}
