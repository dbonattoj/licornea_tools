#ifndef LICORNEA_JSON_H_
#define LICORNEA_JSON_H_

#include <json.hpp>
#include <string>
#include <iosfwd>
#include "opencv.h"

namespace tlz {

using json = nlohmann::json;

void export_json_file(const json&, const std::string& filename);
json import_json_file(const std::string& filename);

cv::Mat_<real> decode_mat(const json&);

json encode_mat_(const cv::Mat_<real>&);

template<typename Mat> json encode_mat(const Mat& mat) {
	return encode_mat_(cv::Mat_<real>(mat));
}

}

#endif
