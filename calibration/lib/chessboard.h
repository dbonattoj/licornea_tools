#ifndef LICORNEA_CHESSBOARD_H_
#define LICORNEA_CHESSBOARD_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <iosfwd>
#include <vector>

#include "calibration_correspondence.h"

struct chessboard_corner {
	float pixel_x;
	float pixel_y;
	int row;
	int col;
};

struct chessboard {
	std::vector<chessboard_corner> corners;
	int rows;
	int cols;
	float real_width;
};

std::vector<calibration_correspondence> calibration_correspondences(const chessboard&);

#endif
