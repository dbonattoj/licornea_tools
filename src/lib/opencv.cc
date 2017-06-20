#include "opencv.h"

namespace tlz {

void cv_aa_circle(cv::Mat& mat, const cv::Point2f& center, float rad, cv::Scalar col, int thickness) {
	int shift = 8;
	int factor = (1 << shift);
	int x_int = center.x * factor, y_int = center.y * factor, rad_int = rad * factor;
	cv::circle(mat, cv::Point(x_int, y_int), rad_int, col, thickness, CV_AA, shift);
}

}
