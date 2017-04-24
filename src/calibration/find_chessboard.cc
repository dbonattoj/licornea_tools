#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "lib/chessboard.h"
#include "lib/calibration_correspondence.h"
#include "../lib/json.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: find_chessboard image.png out_correspondences.json rows cols real_width [out_image.png]\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string in_image_filename = argv[1];
	std::string out_cors_filename = argv[2];
	int rows = std::atoi(argv[3]);
	int cols = std::atoi(argv[4]);
	std::string out_image_filename;
	float real_width = std::atof(argv[5]);
	if(argc > 6) out_image_filename = argv[6];
	
	cv::Mat image = cv::imread(in_image_filename);
	std::vector<cv::Point2f> corners;
	cv::Size pattern_size(cols, rows);
	
	int flags = 0;
	bool found = cv::findChessboardCorners(image, pattern_size, corners, flags);
	
	if(! out_image_filename.empty()) {
		cv::Mat out_image(image.clone());
		cv::drawChessboardCorners(out_image, pattern_size, corners, found);
		cv::imwrite(out_image_filename, out_image);
	}
	
	if(! found) {
		std::cout << "no chessboard corners found" << std::endl;
		return EXIT_FAILURE;
	}
	
	{
		cv::Mat image_gray;
		cv::cvtColor(image, image_gray, CV_BGR2GRAY);
		cv::TermCriteria term(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1);
		cv::cornerSubPix(image_gray, corners, cv::Size(3, 3), cv::Size(-1, -1), term);
	}
	
	{
		int row = 0, col = 0;
		chessboard board;
		board.rows = rows;
		board.cols = cols;
		board.real_width = real_width;
		for(const cv::Point2f& corner : corners) {
			chessboard_corner board_corner;
			board_corner.pixel_x = corner.x;
			board_corner.pixel_y = corner.y;
			board_corner.row = row;
			board_corner.col = col;
			board.corners.push_back(board_corner);
			col++;
			if(col >= cols) { col = 0; row++; }
		}
		
		auto cors = calibration_correspondences(board);
		export_json_file(encode_calibration_correspondences(cors.begin(), cors.end()), out_cors_filename);
	}
	
	
	std::cout << "done" << std::endl;
}

