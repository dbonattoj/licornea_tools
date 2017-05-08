#include "chessboard.h"

namespace tlz {

std::vector<calibration_correspondence> calibration_correspondences(const chessboard& board) {
	std::vector<calibration_correspondence> cors;
	for(const chessboard_corner& corner : board.corners) {
		calibration_correspondence cor;
		cor.object_coordinates = vec3(
			corner.col * board.real_width,
			(board.rows - 1 - corner.row) * board.real_width,
			0.0
		);
		cor.image_coordinates = vec2(corner.pixel_x, corner.pixel_y);
		cors.push_back(cor);
	}
	return cors;
}

}
