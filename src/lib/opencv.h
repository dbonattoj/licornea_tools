#ifndef LICORNEA_OPENCV_H_
#define LICORNEA_OPENCV_H_

#include <opencv2/opencv.hpp>

namespace tlz {

void cv_aa_circle(cv::Mat& mat, const cv::Point2f& center, float rad, cv::Scalar col, int thickness);

#ifdef CV_MAJOR_VERSION
#if CV_MAJOR_VERSION == 3
#define HAVE_OPENCV3
#endif
#endif

}

#endif
