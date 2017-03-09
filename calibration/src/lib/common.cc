#include "common.h"
#include <fstream>

void export_json_file(const json& j, const std::string& filename) {
	std::ofstream output(filename);
	output << j.dump(4);
}


json import_json_file(const std::string& filename) {
	json j;
	std::ifstream input(filename);
	input >> j;
	return j;
}


cv::Mat_<double> decode_mat(const json& j) {
	int rows = j.size();
	int cols = j[0].size();
	cv::Mat_<double> mat(rows, cols);
	for(int row = 0; row < rows; ++row) for(int col = 0; col < cols; ++col)
		mat(row, col) = j[row][col].get<double>();
	return mat;
}


json encode_mat(const cv::Mat_<double>& mat) {
	json j = json::array();
	for(int row = 0; row < mat.rows; ++row) {
		json j_row = json::array();
		for(int col = 0; col < mat.cols; ++col) j_row.push_back(mat(row, col));
		j.push_back(j_row);
	}
	return j;
}
