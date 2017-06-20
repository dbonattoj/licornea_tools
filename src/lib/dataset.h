#ifndef LICORNEA_DATASET_H_
#define LICORNEA_DATASET_H_

#include <string>
#include <utility>
#include <iosfwd>
#include "border.h"
#include "json.h"
#include "border.h"
#include "args.h"

namespace tlz {

class dataset;

struct view_index : index_2d {
	bool is_valid() const { return (x != -1); }
	explicit operator bool () const { return is_valid(); }
	
	bool is_1d() const { return (y == -1); }
	bool is_2d() const { return (y != -1); }

	explicit view_index(int x_ = -1, int y_ = -1) : index_2d(x_, y_) { }
};

std::string encode_view_index(view_index idx);
view_index decode_view_index(const std::string& key);

std::ostream& operator<<(std::ostream&, const view_index&);


class dataset_view {
private:
	const dataset& dataset_;
	int x_;
	int y_;
	std::string group_;

	int local_filename_x_() const;
	int local_filename_y_() const;
	std::string format_name(const std::string& tpl) const;
	std::string format_filename(const std::string& tpl) const;

public:
	dataset_view(const dataset&, int x, int y, const std::string& grp = "");
	
	const dataset& set() const { return dataset_; }
	
	int x() const { return x_; }
	int y() const { return y_; }
	
	const json& local_parameters() const;
	std::string local_filename(const std::string& name, const std::string& def = "") const;
	
	std::string camera_name() const;
	
	std::string image_filename() const;
	std::string depth_filename() const;
	std::string mask_filename() const;

	std::string group() const;
	dataset_view group_view(const std::string& name) const;
};


class dataset_group {
private:
	const dataset& dataset_;
	std::string group_;

public:
	dataset_group(const dataset&, const std::string& grp);
	
	const dataset& set() const { return dataset_; }
	
	const json& parameters() const;
	const json& operator[](const std::string& key) const
		{ return parameters()[key]; }
	
	border image_border() const;
	cv::Size image_size_with_border() const;
	
	dataset_view view(int x) const;
	dataset_view view(int x, int y) const;
	dataset_view view(view_index) const;
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
	const json& operator[](const std::string& key) const
		{ return parameters()[key]; }
	
	std::string filepath(const std::string& relpath) const;
	
	int image_width() const;
	int image_height() const;
	cv::Size image_size() const;
	std::string cameras_filename() const;
	
	int x_min() const;
	int x_step() const;
	int x_max() const;
	bool x_valid(int x) const;
	int x_count() const;
	int x_mid() const;
	std::vector<int> x_indices() const;
	
	int y_min() const;
	int y_step() const;
	int y_max() const;
	bool y_valid(int y) const; 
	int y_count() const;
	int y_mid() const;
	std::vector<int> y_indices() const;
	
	bool valid(view_index) const;
	std::vector<view_index> indices() const;

	dataset_group group(const std::string& grp) const;

	dataset_view view(int x) const;
	dataset_view view(int x, int y) const;
	dataset_view view(view_index) const;
};

dataset dataset_arg();

}

#endif
