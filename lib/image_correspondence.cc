#include "image_correspondence.h"
#include "utility/string.h"

namespace tlz {

image_correspondence_feature decode_image_correspondence_feature(const json& j_feat) {
	image_correspondence_feature feat;
	
	if(j_feat.count("depth") == 1) feat.depth = j_feat["depth"];

	const json& j_pts = j_feat["points"];
	for(auto it = j_pts.begin(); it != j_pts.end(); ++it) {
		std::string key = it.key();	
		image_correspondence_feature::view_index_type idx;
		auto j_idx = explode_from_string<int>(',', key);
		idx.first = j_idx[0];
		if(j_idx.size() == 2)  idx.second = j_idx[1];
		else idx.second = -1;
		
		const json& j_pos = it.value();
		Eigen_vec2 pos(j_pos[0], j_pos[1]);

		feat.points[idx] = pos;
	}
	
	return feat;
}


json encode_image_correspondence_feature(const image_correspondence_feature& feat) {
	json j_pts = json::object();
	for(const auto& pt : feat.points) {
		image_correspondence_feature::view_index_type idx = pt.first;
		std::string key = std::to_string(idx.first);
		if(idx.second != -1) key += "," + std::to_string(idx.second);
		
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
