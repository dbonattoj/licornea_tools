/*
#include "old_dataset.h"
#include <format.h>
#include <fstream>
#include <stdexcept>

namespace tlz {

dataset::dataset(const std::string& filename) {
	std::size_t last_sep_pos = filename.find_last_of('/');
	if(last_sep_pos == std::string::npos) dirname_ = "./";
	else dirname_ = filename.substr(0, last_sep_pos + 1);
	
	
	std::ifstream str(filename);
	str >> json_;
	is_2d_ = (json_["arrangement"].count("y_index_range") == 1);
	
}

bool dataset::has_kinect_raw() const {
	return (json_["arrangement"].count("kinect_raw") == 1);
}

int dataset::x_min() const {
	return json_["arrangement"]["x_index_range"][0];
}

int dataset::x_max() const {
	return json_["arrangement"]["x_index_range"][1];
}

int dataset::x_step() const {
	if(json_["arrangement"]["x_index_range"].size() == 3) return json_["arrangement"]["x_index_range"][2];
	else return 1;
}

std::vector<int> dataset::x_indices() const {
	std::vector<int> indices;
	for(int x = x_min(); x <= x_max(); x += x_step()) indices.push_back(x);
	return indices;
}


int dataset::y_min() const {
	if(is_2d_) return json_["arrangement"]["y_index_range"][0];
	else return 0;
}

int dataset::y_max() const {
	if(is_2d_) return json_["arrangement"]["y_index_range"][1];
	else return 0;
}

int dataset::y_step() const {
	if(is_2d_ && json_["arrangement"]["y_index_range"].size() == 3) return json_["arrangement"]["y_index_range"][2];
	else return 1;
}

std::vector<int> dataset::y_indices() const {
	if(! is_2d_) return {0};
	
	std::vector<int> indices;
	for(int y = y_min(); y <= y_max(); y += y_step()) indices.push_back(y);
	return indices;
}

dataset_view dataset::view(int x) const {
	if(is_2d_) throw std::logic_error("need 2 indices on 2D dataset");
	if(x < x_min() || x > x_max() || (x - x_min()) % x_step() != 0) throw std::out_of_range("dataset x index out of range");
	return dataset_view(*this, x, 0);
}

dataset_view dataset::view(int x, int y) const {
	if(x < x_min() || x > x_max() || (x - x_min()) % x_step() != 0) throw std::out_of_range("dataset x index out of range");
	if(y < y_min() || y > y_max() || (y - y_min()) % y_step() != 0) throw std::out_of_range("dataset y index out of range");	
	return dataset_view(*this, x, y);
}

int dataset_view::kinect_raw_x_() const {
	double factor = 1.0;
	int offset = 0;
	if(dataset_.get_json()["arrangement"]["kinect_raw"].count("x_index_factor") == 1)
		factor = dataset_.get_json()["arrangement"]["kinect_raw"]["x_index_factor"].get<double>();
	if(dataset_.get_json()["arrangement"]["kinect_raw"].count("x_index_offset") == 1)
		offset = dataset_.get_json()["arrangement"]["kinect_raw"]["x_index_offset"].get<int>();

	int raw_x = x_;
	raw_x *= factor;
	raw_x += offset;
	return raw_x;
}

int dataset_view::kinect_raw_y_() const {
	if(! dataset_.is_2d()) throw std::logic_error("dataset must be 2d for kinect_raw_y_()");

	double factor = 1.0;
	int offset = 0;
	if(dataset_.get_json()["arrangement"]["kinect_raw"].count("y_index_factor") == 1)
		factor = dataset_.get_json()["arrangement"]["kinect_raw"]["y_index_factor"].get<double>();
	if(dataset_.get_json()["arrangement"]["kinect_raw"].count("y_index_offset") == 1)
		offset = dataset_.get_json()["arrangement"]["kinect_raw"]["y_index_offset"].get<int>();

	int raw_y = y_;
	raw_y *= factor;
	raw_y += offset;
	return raw_y;
}

std::string dataset_view::yuv_texture_filename() const {
	const std::string& format = dataset_.get_json()["arrangement"]["texture_filename_format"];
	if(dataset_.is_2d()) return dataset_.dirname() + fmt::format(format, fmt::arg("x", x_), fmt::arg("y", y_));
	else return dataset_.dirname() + fmt::format(format, fmt::arg("x", x_));		
}

std::string dataset_view::yuv_depth_filename() const {
	const std::string& format = dataset_.get_json()["arrangement"]["depth_filename_format"];
	if(dataset_.is_2d()) return dataset_.dirname() + fmt::format(format, fmt::arg("x", x_), fmt::arg("y", y_));
	else return dataset_.dirname() + fmt::format(format, fmt::arg("x", x_));		
}

std::string dataset_view::raw_texture_filename() const {
	if(! dataset_.has_kinect_raw()) throw std::logic_error("dataset must have kinect_raw");
	const std::string& format = dataset_.get_json()["arrangement"]["kinect_raw"]["texture_filename_format"];
	if(dataset_.is_2d()) return dataset_.dirname() + fmt::format(format, fmt::arg("x", kinect_raw_x_()), fmt::arg("y", kinect_raw_y_()));
	else return dataset_.dirname() + fmt::format(format, fmt::arg("x", kinect_raw_x_()));		
}

std::string dataset_view::raw_depth_filename() const {
	if(! dataset_.has_kinect_raw()) throw std::logic_error("dataset must have kinect_raw");
	const std::string& format = dataset_.get_json()["arrangement"]["kinect_raw"]["depth_filename_format"];
	if(dataset_.is_2d()) return dataset_.dirname() + fmt::format(format, fmt::arg("x", kinect_raw_x_()), fmt::arg("y", kinect_raw_y_()));
	else return dataset_.dirname() + fmt::format(format, fmt::arg("x", kinect_raw_x_()));
}


}
*/
