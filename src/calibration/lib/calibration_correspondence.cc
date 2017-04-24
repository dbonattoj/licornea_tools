#include "calibration_correspondence.h"

namespace tlz {

calibration_correspondence decode_calibration_correspondence(const json& j) {
	calibration_correspondence cor;
	for(int i = 0; i < 3; ++i) cor.object_coordinates[i] = j["object"][i].get<double>();
	for(int i = 0; i < 2; ++i) cor.image_coordinates[i] = j["image"][i].get<double>();
	return cor;
}


json encode_calibration_correspondence(const calibration_correspondence& cor) {
	json j = json::object();
	for(int i = 0; i < 3; ++i) j["object"][i] = cor.object_coordinates[i];
	for(int i = 0; i < 2; ++i) j["image"][i] = cor.image_coordinates[i];
	return j;
} 

}
