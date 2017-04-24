#include "dataset.h"
#include "utility/string.h"
#include <format.h>
#include <stdexcept>
#include <fstream>
#include <string>

namespace tlz {

const json& dataset_view::local_parameters_() const {
	if(files_group_.empty()) return dataset_.parameters();
	else return dataset_.parameters()[files_group_];
}

int dataset_view::local_filename_x_() const {
	float factor = 1.0;
	int offset = 0;
	if(local_parameters_().count("filename_x_index_factor") == 1) factor = local_parameters_()["filename_x_index_factor"];
	if(local_parameters_().count("filename_x_index_offset") == 1) offset = local_parameters_()["filename_x_index_offset"];

	int loc_x = x_;
	loc_x *= factor;
	loc_x += offset;
	return loc_x;
}

int dataset_view::local_filename_y_() const {
	float factor = 1.0;
	int offset = 0;
	if(local_parameters_().count("filename_y_index_factor") == 1) factor = local_parameters_()["filename_y_index_factor"];
	if(local_parameters_().count("filename_y_index_offset") == 1) offset = local_parameters_()["filename_y_index_offset"];

	int loc_y = y_;
	loc_y *= factor;
	loc_y += offset;
	return loc_y;
}

dataset_view dataset_view::local_view_(const std::string& name) const {
	if(dataset_.parameters().count(name) == 0) throw std::runtime_error("no local view '" + name + "' in parameters");
	return dataset_view(dataset_, x_, y_, name);
}

std::string dataset_view::format_name(const std::string& tpl) const {
	if(dataset_.is_2d()) return fmt::format(tpl, fmt::arg("x", x_), fmt::arg("y", y_));
	else return fmt::format(tpl, fmt::arg("x", x_));
}

std::string dataset_view::format_filename(const std::string& tpl) const {
	std::string relpath;
	if(dataset_.is_2d()) relpath = fmt::format(tpl, fmt::arg("x", local_filename_x_()), fmt::arg("y", local_filename_y_()));
	else relpath =  fmt::format(tpl, fmt::arg("x", local_filename_x_()));
	return dataset_.filepath(relpath);
}

dataset_view::dataset_view(const dataset& datas, int x, int y, const std::string& files_group) :
	dataset_(datas), x_(x), y_(y), files_group_(files_group) { }

std::string dataset_view::local_string_parameter(const std::string& name, const std::string& def) const {
	auto j = local_parameters_();
	if(j.count(name) == 1) return j[name];
	else return def;
}
 
std::string dataset_view::local_filename(const std::string& name, const std::string& def) const {
	std::string tpl = local_string_parameter(name, "");
	if(! tpl.empty()) return format_filename(tpl);
	else return def;
}

std::string dataset_view::camera_name() const {	
	return format_name(dataset_.parameters()["camera_name_format"]);
}

std::string dataset_view::image_filename() const {
	return local_filename("image_filename_format");
}

std::string dataset_view::depth_filename() const {
	return local_filename("depth_filename_format");
}

std::string dataset_view::mask_filename() const {
	return local_filename("mask_filename_format");
}

dataset_view dataset_view::vsrs() const {
	return local_view_("vsrs");
}

dataset_view dataset_view::kinect_raw() const {
	return local_view_("kinect_raw");
}
	
/////


dataset::dataset(const std::string& parameters_filename) {
	std::size_t last_sep_pos = parameters_filename.find_last_of('/');
	if(last_sep_pos == std::string::npos) dirname_ = "./";
	else dirname_ = parameters_filename.substr(0, last_sep_pos + 1);
	
	std::ifstream stream(parameters_filename);
	stream >> parameters_;

	for(int v : parameters_["x_index_range"]) x_index_range_.push_back(v);
	if(parameters_.count("y_index_range") == 1)
		for(int v : parameters_["y_index_range"]) y_index_range_.push_back(v);
}

bool dataset::is_1d() const {
	return (y_index_range_.size() == 0);
}

bool dataset::is_2d() const {
	return (y_index_range_.size() > 0);
}

std::string dataset::filepath(const std::string& relpath) const {
	return dirname_ + relpath;
}

std::string dataset::cameras_filename() const {
	return filepath(parameters_["cameras_filename"]);
}

int dataset::x_min() const {
	return x_index_range_[0];
}

int dataset::x_max() const {
	return x_index_range_[1];
}

int dataset::x_step() const {
	if(x_index_range_.size() == 3) return x_index_range_[2];
	else return 1;
}

bool dataset::x_valid(int x) const {
	return (x >= x_min()) && (x <= x_max()) && (((x - x_min()) % x_step()) == 0);
}

std::vector<int> dataset::x_indices() const {
	std::vector<int> indices;
	for(int x = x_min(); x <= x_max(); x += x_step()) indices.push_back(x);
	return indices;
}

int dataset::y_min() const {
	if(is_2d()) return y_index_range_[0];
	else return 0;
}

int dataset::y_max() const {
	if(is_2d()) return y_index_range_[1];
	else return 0;
}

int dataset::y_step() const {
	if(is_2d() && (y_index_range_.size() == 3)) return y_index_range_[2];
	else return 1;
}

bool dataset::y_valid(int y) const {
	return (y >= y_min()) && (y <= y_max()) && (((y - y_min()) % y_step()) == 0);
}

std::vector<int> dataset::y_indices() const {
	std::vector<int> indices;
	for(int y = y_min(); y <= y_max(); y += y_step()) indices.push_back(y);
	return indices;
}

dataset_view dataset::view(int x) const {
	if(is_2d()) throw std::runtime_error("must specify y view index, for 2d dataset");
	if(! x_valid(x)) throw std::runtime_error("x view index out of range");
	return dataset_view(*this, x, 0);
}

dataset_view dataset::view(int x, int y) const {
	if(! x_valid(x)) throw std::runtime_error("x view index out of range");
	if(! y_valid(y)) throw std::runtime_error("y view index out of range");
	return dataset_view(*this, x, y);
}

dataset_view dataset::view(view_index idx) const {
	if(is_2d()) {
		if(idx.second == -1) throw std::runtime_error("must specify 2d view index");
		return view(idx.first, idx.second);
	} else {
		if(idx.second != -1) throw std::runtime_error("must specify 1d view index");
		return view(idx.first);
	}
}


//////////


view_index make_view_index_1d(int x) {
	return view_index(x, -1);
}

view_index make_view_index_2d(int x, int y) {
	return view_index(x, y);
}

std::string view_index_to_key(view_index idx) {
	std::string key = std::to_string(idx.first);
	if(idx.second != -1) key += "," + std::to_string(idx.second);
	return key;
}

view_index view_index_from_key(const std::string& key) {
	view_index idx;
	auto j_idx = explode_from_string<int>(',', key);
	idx.first = j_idx[0];
	if(j_idx.size() == 2)  idx.second = j_idx[1];
	else idx.second = -1;
	return idx;
}


}
