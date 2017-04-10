#include "img2img_correspondence.h"

namespace tlz {

img2img_correspondence decode_img2img_correspondence(const json& j_cor) {
	img2img_correspondence cor;
	for(const json& j_pt : j_cor) {
		int x_idx = j_pt["view"]["x"];
		int y_idx = -1;
		if(j_pt["view"].count("y") == 1) y_idx = j_pt["view"]["y"];
		cv::Vec2f pos(j_pt["position"][0], j_pt["position"][1]);
		img2img_correspondence::view_index_type idx(x_idx, y_idx);
		cor.images_coordinates[idx] = pos;
	}
	return cor;
}


json encode_img2img_correspondence(const img2img_correspondence& cor) {
	json j_cor = json::array();
	for(const auto& pt : cor.images_coordinates) {
		img2img_correspondence::view_index_type idx = pt.first;
		cv::Vec2f pos = pt.second;
		
		json j_pt = json::object();
		
		json j_pt_view = json::object();
		j_pt_view["x"] = idx.first;
		if(idx.second != -1) j_pt_view["y"] = idx.second;
		j_pt["view"] = j_pt_view;
		
		j_pt["position"] = json::array();
		j_pt["position"].push_back(pos[0]);
		j_pt["position"].push_back(pos[1]);
		
		j_cor.push_back(j_pt);
	}
	return j_cor;
}


}
