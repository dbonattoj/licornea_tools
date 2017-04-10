#ifndef LICORNEA_CALIB_IMG2IMG_CORRESPONDENCE_H_
#define LICORNEA_CALIB_IMG2IMG_CORRESPONDENCE_H_

#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <map>
#include "../../lib/json.h"

namespace tlz {


struct img2img_correspondence {	
	using view_index_type = std::pair<int, int>;
	// when iterating (--> encoding into json array), gets sorted by index
	std::map<view_index_type, cv::Vec2f> images_coordinates;
};

img2img_correspondence decode_img2img_correspondence(const json&);
json encode_img2img_correspondence(const img2img_correspondence&); 

/*
template<typename It>
void decode_img2img_correspondences(const json& j, It it) {
	for(const json& j_cor : j)
		*it = decode_img2img_correspondence(j_cor);
}

template<typename It>
json encode_img2img_correspondences(It begin, It end) {
	json j = json::array();
	for(It it = begin; it != end; ++it)
		j.push_back(encode_img2img_correspondence(*it));
	return j;
}
*/

}

#endif
