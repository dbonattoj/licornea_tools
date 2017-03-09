#ifndef LICORNEA_COMMON_H_
#define LICORNEA_COMMON_H_

#include <json.hpp>
#include <opencv2/opencv.hpp>
#include <string>

using json = nlohmann::json;

void export_json_file(const json&, const std::string& filename);
json import_json_file(const std::string& filename);

cv::Mat_<double> decode_mat(const json&);
json encode_mat(const cv::Mat_<double>&);

#endif
