#include "chessboard.h"
#include <json.hpp>
#include <istream>
#include <ostream>

chessboard decode_chessboard(const json& j_root) {
	chessboard board;
	
	board.rows = j_root["rows"];
	board.cols = j_root["cols"];
	board.real_width = j_root["real_width"];
	
	for(const json& j_corner : j_root["corners"]) {
		chessboard_corner corner;
		corner.pixel_x = j_corner["pixel_x"];
		corner.pixel_y = j_corner["pixel_y"];
		corner.row = j_corner["row"];
		corner.col = j_corner["col"];
		board.corners.push_back(corner);
	}
	
	return board;
}


json encode_chessboard(const chessboard& board) {	
	json j_root = json::object();
		
	j_root["rows"] = board.rows;
	j_root["cols"] = board.cols;
	j_root["real_width"] = board.real_width;
	
	json j_corners = json::array();		
	for(const chessboard_corner& corner : board.corners) {
		json j_corner = json::object();
		j_corner["pixel_x"] = corner.pixel_x;
		j_corner["pixel_y"] = corner.pixel_y;
		j_corner["row"] = corner.row;
		j_corner["col"] = corner.col;
		j_corners.push_back(j_corner);
	}
	j_root["corners"] = j_corners;
	return j_root;
}


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
