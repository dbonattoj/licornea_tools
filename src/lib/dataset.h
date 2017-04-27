#ifndef LICORNEA_DATASET_H_
#define LICORNEA_DATASET_H_

#include "json.h"
#include <string>
#include <utility>

namespace tlz {

class dataset;

struct view_index {
	int x;
	int y;
	
	bool is_valid() const { return (x != -1); }
	explicit operator bool () const { return is_valid(); }
	
	bool is_1d() const { return (y == -1); }
	bool is_2d() const { return (y != -1); }

	view_index() : x(-1), y(-1) { }
	explicit view_index(int x_, int y_ = -1) : x(x_), y(y_) { }
};

inline bool operator==(const view_index& a, const view_index& b) {
	return (a.y == b.y) && (a.x == b.x);
}
inline bool operator<(const view_index& a, const view_index& b) {
	return (a.y < b.y) || (a.x < b.x);
}

std::string encode_view_index(view_index idx);
view_index decode_view_index(const std::string& key);


class dataset_view {
private:
	const dataset& dataset_;
	int x_;
	int y_;
	std::string files_group_;

	const json& local_parameters_() const;
	int local_filename_x_() const;
	int local_filename_y_() const;
	dataset_view local_view_(const std::string& name) const;
	std::string format_name(const std::string& tpl) const;
	std::string format_filename(const std::string& tpl) const;

public:
	dataset_view(const dataset&, int x, int y, const std::string& files_group = "");
	
	int x() const { return x_; }
	int y() const { return y_; }
	
	std::string local_string_parameter(const std::string& name, const std::string& def = "") const;
	std::string local_filename(const std::string& name, const std::string& def = "") const;
	
	std::string camera_name() const;
	
	std::string image_filename() const;
	std::string depth_filename() const;
	std::string mask_filename() const;
	
	dataset_view vsrs() const;
	dataset_view kinect_raw() const;
};

class dataset {	
private:
	json parameters_;
	std::string dirname_;
	std::vector<int> x_index_range_;
	std::vector<int> y_index_range_;
	
public:
	explicit dataset(const std::string& parameters_filename);
	
	bool is_1d() const;
	bool is_2d() const;

	const json& parameters() const { return parameters_; }
	std::string filepath(const std::string& relpath) const;
	
	std::string cameras_filename() const;
	
	int x_min() const;
	int x_step() const;
	int x_max() const;
	bool x_valid(int x) const;
	std::vector<int> x_indices() const;
	
	int y_min() const;
	int y_step() const;
	int y_max() const;
	bool y_valid(int y) const; 
	std::vector<int> y_indices() const;

	dataset_view view(int x) const;
	dataset_view view(int x, int y) const;
	dataset_view view(view_index) const;
};

}

#endif
