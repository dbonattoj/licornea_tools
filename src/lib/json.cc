#include "json.h"
#include "string.h"
#include <fstream>
#include <vector>
#include <cstdint>

namespace tlz {
	
void export_json_file(const json& j, const std::string& filename) {
	std::string ext = file_name_extension(filename);
	if(ext == "json") {
		std::ofstream output(filename);
		output << j.dump(4);
	} else if(ext == "cbor") {
		std::vector<std::uint8_t> cbor_data = json::to_cbor(j);
		std::ofstream output(filename, std::ios_base::out | std::ios_base::binary);
		output.write(reinterpret_cast<const std::ofstream::char_type*>(cbor_data.data()), cbor_data.size());
	} else {
		throw std::runtime_error("unknown json filename extension");
	}
}


json import_json_file(const std::string& filename) {
	json j;
	std::ifstream input(filename);
	input >> j;
	return j;
}


cv::Mat_<real> decode_mat(const json& j) {
	int rows = j.size();
	if(j[0].is_array()) {
		int cols = j[0].size();
		cv::Mat_<real> mat(rows, cols);
		for(int row = 0; row < rows; ++row) for(int col = 0; col < cols; ++col)
			mat(row, col) = j[row][col].get<real>();
		return mat;
	
	} else {
		cv::Mat_<real> mat(rows, 1);
		for(int row = 0; row < rows; ++row)
			mat(row, 0) = j[row].get<real>();
		return mat;

	}
}


json encode_mat_(const cv::Mat_<real>& mat) {
	json j = json::array();
	if(mat.cols == 1) {
		for(int row = 0; row < mat.rows; ++row) j.push_back(mat(row, 0));
	} else {
		for(int row = 0; row < mat.rows; ++row) {
			json j_row = json::array();
			for(int col = 0; col < mat.cols; ++col) j_row.push_back(mat(row, col));
			j.push_back(j_row);
		}
	}
	return j;
}


}
