#ifndef LICORNEA_KINECT_REPROJECTION_H_
#define LICORNEA_KINECT_REPROJECTION_H_

#include "../../lib/opencv.h"
#include "../../lib/color.h"
#include "kinect_reprojection_parameters.h"
#include "ir_to_color_sample.h"
#include <vector>
#include <tuple>

namespace tlz {


class kinect_reprojection {
private:
	kinect_reprojection_parameters reprojection_parameters_;
	
public:
	explicit kinect_reprojection(const kinect_reprojection_parameters&);
	
	kinect_reprojection_parameters& parameters() { return reprojection_parameters_; }
	const kinect_reprojection_parameters& parameters() const { return reprojection_parameters_; }
	
	std::vector<vec2> reproject_points_ir_to_color(
		const std::vector<vec2> distorted_ir_i_xy,
		const std::vector<real>& ir_z,
		std::vector<real>& out_color_z,
		bool distort_color = true
	) const;
	
	template<typename Value> using sample = ir_to_color_sample<Value>;

	template<typename Value, typename Depth>
	std::vector<sample<Value>> reproject_ir_to_color_samples(
		const cv::Mat_<Value>& distorted_ir_values,
		const cv::Mat_<Depth>& distorted_ir_z,
		bool distort_color = true
	) const;
};

}

#include "kinect_reprojection.tcc"

#endif
