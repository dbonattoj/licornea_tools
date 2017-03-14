#include "depth_densify.h"
#include <vector>

namespace tlz {

void depth_densify(const std::vector<Eigen_vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	out.setTo(0.0);
	out_mask.setTo(0);

	auto cmp = [](const Eigen_vec3& a, const Eigen_vec3& b)->bool { return (a[2] > b[2]); };
	std::vector<Eigen_vec3> samples_sorted = samples;
	std::sort(samples_sorted.begin(), samples_sorted.end(), cmp);
	
	int rad = 4;
	for(const Eigen_vec3& sample : samples_sorted) {
		cv::circle(out, cv::Point(sample[0], sample[1]), rad, sample[2], -1);
		cv::circle(out_mask, cv::Point(sample[0], sample[1]), rad, 0xff, -1);
	}
}

}
