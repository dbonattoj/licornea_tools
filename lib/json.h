#ifndef LICORNEA_JSON_H_
#define LICORNEA_JSON_H_

#include <json.hpp>
#include <string>
#include "opencv.h"

namespace tlz {

using json = nlohmann::json;

void export_json_file(const json&, const std::string& filename);
json import_json_file(const std::string& filename);

cv::Mat_<double> decode_mat(const json&);
json encode_mat(const cv::Mat_<double>&);

}

#endif
