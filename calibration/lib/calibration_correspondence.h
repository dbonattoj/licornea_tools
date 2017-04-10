#ifndef LICORNEA_CALIB_CALIBRATION_CORRESPONDENCES_H_
#define LICORNEA_CALIB_CALIBRATION_CORRESPONDENCES_H_

#include <opencv2/opencv.hpp>
#include <json.hpp>
#include "../../lib/json.h"

namespace tlz {

// TODO rename into img2world or similar

struct calibration_correspondence {
	cv::Vec3f object_coordinates;
	cv::Vec2f image_coordinates;
};

calibration_correspondence decode_calibration_correspondence(const json&);
json encode_calibration_correspondence(const calibration_correspondence&); 

template<typename It>
void decode_calibration_correspondences(const json& j, It it) {
	for(const json& j_cor : j)
		*it = decode_calibration_correspondence(j_cor);
}

template<typename It>
json encode_calibration_correspondences(It begin, It end) {
	json j = json::array();
	for(It it = begin; it != end; ++it)
		j.push_back(encode_calibration_correspondence(*it));
	return j;
}

}

#endif
