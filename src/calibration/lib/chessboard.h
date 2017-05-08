#ifndef LICORNEA_CALIB_CHESSBOARD_H_
#define LICORNEA_CALIB_CHESSBOARD_H_

#include "../../lib/common.h"
#include "../../lib/opencv.h"
#include <vector>
#include <iosfwd>
#include <vector>

#include "calibration_correspondence.h"

namespace tlz {

struct chessboard_corner {
	real pixel_x;
	real pixel_y;
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

}

#endif
