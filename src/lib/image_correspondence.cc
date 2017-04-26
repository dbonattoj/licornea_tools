#include "image_correspondence.h"
#include "utility/string.h"

#include <iostream>

namespace tlz {


image_correspondence_feature decode_image_correspondence_feature(const json& j_feat) {
	image_correspondence_feature feat;
	
	if(j_feat.count("depth") == 1) feat.depth = j_feat["depth"];

	const json& j_pts = j_feat["points"];
	for(auto it = j_pts.begin(); it != j_pts.end(); ++it) {	
		std::string key = it.key();	
		view_index idx = view_index_from_key(key);
		
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
		std::string key = view_index_to_key(idx);
		
		Eigen_vec2 pos = pt.second;
		json j_pos = json::array();
		j_pos.push_back(pos[0]);
		j_pos.push_back(pos[1]);
		
		j_pts[key] = j_pos;
	}
	
	json j_feat = json::object();
	j_feat["points"] = j_pts;
	if(feat.depth != 0.0) j_feat["depth"] = feat.depth;
		
	return j_feat;
}


}