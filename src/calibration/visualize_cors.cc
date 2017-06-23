#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/random_color.h"
#include "../lib/dataset.h"
#include "../lib/border.h"
#include "../lib/opencv.h"
#include "../lib/string.h"

using namespace tlz;

const cv::Vec3b black(0, 0, 0);
const cv::Vec3b white(255, 255, 255);

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json out_visualization.png feature_name [dot_radius=2]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string visualization_filename = out_filename_arg();
	std::string feature_name = string_arg();
	int dot_radius = int_opt_arg(2);
	
	dataset_group datag = datas.group(cors.dataset_group);
	border bord = datag.image_border();

	const image_correspondence_feature& feature = cors.features.at(feature_name);
	view_index reference_idx = feature.reference_view;

	std::cout << "loading reference image" << std::endl;
	cv::Mat_<cv::Vec3b> img;
	{
		std::string reference_image_filename = datag.view(reference_idx).image_filename();
		cv::Mat_<cv::Vec3b> reference_image = cv::imread(reference_image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> reference_gray_img;
		cv::cvtColor(reference_image, reference_gray_img, CV_BGR2GRAY);
		cv::cvtColor(reference_gray_img, img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing image correspondences" << std::endl;	
	cv::Vec3b col = random_color(string_hash(feature_name));
	img = visualize_view_points(feature, img, col, dot_radius, bord);
		
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

