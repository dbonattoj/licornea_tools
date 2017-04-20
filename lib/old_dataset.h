/*
 * #ifndef LICORNEA_DATASET_H_
#define LICORNEA_DATASET_H_

#include "json.h"
#include <string>
#include <utility>

namespace tlz {

class dataset;

class dataset_view {
private:
	const dataset& dataset_;
	int x_;
	int y_;

	int kinect_raw_x_() const;
	int kinect_raw_y_() const;

public:
	dataset_view(const dataset& set, int x, int y) :
		dataset_(set), x_(x), y_(y) { }
	
	int x() const { return x_; }
	int y() const { return y_; }
		
	std::string yuv_texture_filename() const;
	std::string yuv_depth_filename() const;
	std::string raw_texture_filename() const;
	std::string raw_depth_filename() const;
};

class dataset {	
private:
	json json_;
	std::string dirname_;
	bool is_2d_;
	
public:
	explicit dataset(const std::string& filename);
	
	bool is_1d() const { return ! is_2d_; }
	bool is_2d() const { return is_2d_; }
	bool has_kinect_raw() const;
	
	const json& get_json() const { return json_; }
	const std::string& dirname() const { return dirname_; }
	
	int x_min() const;
	int x_step() const;
	int x_max() const;
	std::vector<int> x_indices() const;
	
	int y_min() const;
	int y_step() const;
	int y_max() const;
	std::vector<int> y_indices() const;

	dataset_view view(int x) const;
	dataset_view view(int x, int y) const;
};

}

#endif
*/
