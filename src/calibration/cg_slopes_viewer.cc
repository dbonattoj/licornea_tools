#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/utility/misc.h"
#include "lib/cg/feature_points.h"
#include "lib/cg/feature_slopes.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <functional>

using namespace tlz;

constexpr real pi = 3.14159265359;
constexpr real deg_per_rad = 180.0 / pi;
constexpr real rad_per_deg = pi / 180.0;

constexpr real comp_abs_max = 30.0 * rad_per_deg;
constexpr int comp_slider_max = 1000;

const cv::Vec3b label_color(255, 255,255);
const cv::Vec3b background_color(0, 0, 0);

std::function<void()> update_function;

mat33 to_rotation_matrix(real x, real y, real z) {
	mat33 Rx(
		1.0, 0.0, 0.0,
		0.0, std::cos(x), -std::sin(x),
		0.0, std::sin(x), std::cos(x)
	);
	mat33 Ry(
		std::cos(y), 0.0, std::sin(y),
		0.0, 1.0, 0.0,
		-std::sin(y), 0.0, std::cos(y)
	);
	mat33 Rz(
		std::cos(z), -std::sin(z), 0.0,
		std::sin(z), std::cos(z), 0.0,
		0.0, 0.0, 1.0
	);
	mat33 R = Rz * Ry * Rx;
	return R.t();
}

void update_callback(int = 0, void* = nullptr) {	
	update_function();
}


int to_int(real val) {
	return ((val / comp_abs_max) + 1.0) * comp_slider_max / 2.0;
}
real from_int(int ival) {
	return ((2.0 * ival / comp_slider_max) - 1.0) * comp_abs_max;
}
 

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_slopes_viewer dataset_parameters.json intrinsics.json points.json/measured_slopes.json\n" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameters_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string point_or_slopes_filename = argv[3];
	
	std::cout << "loading dataset and intrinsics" << std::endl;
	dataset datas(dataset_parameters_filename);
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));

	std::cout << "loading feature points or slopes" << std::endl;
	feature_points fpoints;
	feature_slopes measured_fslopes;
	bool has_measured_slopes;
	{
		json j = import_json_file(point_or_slopes_filename);
		has_measured_slopes = has_feature_slopes(j);
		if(has_measured_slopes) fpoints = measured_fslopes = decode_feature_slopes(j);
		else fpoints = decode_feature_points(j);
	}

	std::cout << "preparing background image" << std::endl;
	cv::Mat_<cv::Vec3b> back_image;
	int width, height;
	{
		view_index view_index = fpoints.view_idx;
		std::string image_filename = datas.view(view_index).image_filename();
		cv::Mat_<cv::Vec3b> img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
	
		cv::Mat_<cv::Vec3b> undist_img;
		cv::undistort(
			img,
			undist_img,
			intr.K,
			intr.distortion.cv_coeffs(),
			intr.K
		);
		
		back_image = undist_img;
		width = back_image.cols;
		height = back_image.rows;
	}

	
	std::cout << "running viewer" << std::endl;

	const std::string window_name = "Slopes Viewer";

	int x_int = comp_slider_max / 2;
	int y_int = comp_slider_max / 2;
	int z_int = comp_slider_max / 2;
	int measured_width = 0;
	int model_width = 200;
	int exaggeration_int = 100;
	
	real herror = NAN;
	real verror = NAN;
	real error = NAN;

	cv::Mat_<cv::Vec3b> viz_image(height, width);
	cv::Mat_<cv::Vec3b> shown_image(height+20+(has_measured_slopes ? 20 : 0), width);
	auto update = [&]() {
		// parameters
		real x = from_int(x_int);
		real y = from_int(y_int);
		real z = from_int(z_int);
		real exaggeration = exaggeration_int / 100.0;
				
		// draw background
		back_image.copyTo(viz_image);

		// draw measured slopes
		if(has_measured_slopes && measured_width > 0)
			viz_image = visualize_feature_slopes(measured_fslopes, viz_image, measured_width, exaggeration, 2);

		// make grayscale
		{
			cv::Mat interm;
			cv::cvtColor(viz_image, interm, CV_BGR2GRAY);
			cv::cvtColor(interm, viz_image, CV_GRAY2BGR);
		}

		// draw points
		//viz_image = visualize_feature_points(fpoints, viz_image);

		// compute and draw model slopes
		feature_slopes model_fslopes(fpoints);
		if(model_width > 0) {			
			mat33 R = to_rotation_matrix(x, y, z);
	
			for(const auto& kv : fpoints.points) {
				const std::string& feature_name = kv.first;
				const feature_point& fpoint = kv.second;
				feature_slope& fslope = model_fslopes.slopes[feature_name];
				fslope.horizontal = model_horizontal_slope(fpoint.undistorted_point, intr.K, R);
				fslope.vertical = model_vertical_slope(fpoint.undistorted_point, intr.K, R);
			}

			viz_image = visualize_feature_slopes(model_fslopes, viz_image, model_width, exaggeration, 1);
		}
		
		shown_image.setTo(background_color);
		viz_image.copyTo(cv::Mat(shown_image, cv::Rect(0, 20, width, height)));

		if(has_measured_slopes) {
			herror = 0;
			verror = 0;
			for(const auto& kv : fpoints.points) {
				const std::string& feature_name = kv.first;
				const feature_slope& measured_fslope = measured_fslopes.slopes.at(feature_name);
				const feature_slope& model_fslope = model_fslopes.slopes.at(feature_name);
				
				herror += sq(measured_fslope.horizontal - model_fslope.horizontal);
				verror += sq(measured_fslope.vertical - model_fslope.vertical);
			}
			int n = fpoints.points.size();
			real error_coeff = 100.0;
			herror /= n;
			verror /= n;
			error = herror + verror;
			error *= error_coeff;
			herror *= error_coeff;
			verror *= error_coeff;
		}

		// draw label
		int font = cv::FONT_HERSHEY_COMPLEX_SMALL;
		double fontscale = 0.7;
		int thickness = 1;
		std::string label = "x=" + std::to_string(x * deg_per_rad) + "    y=" + std::to_string(y * deg_per_rad) + "    z=" + std::to_string(z * deg_per_rad);
		cv::putText(shown_image, label, cv::Point(10, 13), font, fontscale, cv::Scalar(label_color), thickness);

		if(has_measured_slopes) {
			label = "herror=" + std::to_string(herror) + "    verror=" + std::to_string(verror) + "    error=" + std::to_string(error);
			cv::putText(shown_image, label, cv::Point(10, 20+height+13), font, fontscale, cv::Scalar(label_color), thickness);
		}

		cv::imshow(window_name, shown_image);
	};
	update_function = update;

	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	cv::createTrackbar("X", window_name, &x_int, comp_slider_max, &update_callback);
	cv::createTrackbar("Y", window_name, &y_int, comp_slider_max, &update_callback);
	cv::createTrackbar("Z", window_name, &z_int, comp_slider_max, &update_callback);
	if(has_measured_slopes) cv::createTrackbar("measured width", window_name, &measured_width, 400, &update_callback);
	cv::createTrackbar("model width", window_name, &model_width, 400, &update_callback);
	cv::createTrackbar("exaggeration", window_name, &exaggeration_int, 1000, &update_callback);

	update();
	
	cv::waitKey(0);
}
