#include "chessboard.h"
#include <istream>
#include <ostream>

std::vector<calibration_correspondence> calibration_correspondences(const chessboard& board) {
	std::vector<calibration_correspondence> cors;
	for(const chessboard_corner& corner : board.corners) {
		calibration_correspondence cor;
		cor.object_coordinates = cv::Vec3f(
			corner.col * board.real_width,
			(board.rows - 1 - corner.row) * board.real_width,
			0.0
		);
		cor.image_coordinates = cv::Vec2f(corner.pixel_x, corner.pixel_y);
		cors.push_back(cor);
	}
	return cors;
}
