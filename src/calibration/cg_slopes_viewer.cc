#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "../lib/assert.h"
#include "lib/feature_points.h"
#include "lib/cg/feature_slopes.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;

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

 

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json intrinsics.json points.json/measured_slopes.json");
	dataset datas = dataset_arg();
	intrinsics intr = intrinsics_arg();
	std::string point_or_slopes_filename = in_filename_arg();
	std::string dataset_group_name = ""; // TODO include with feature_points

	Assert(intr.distortion.is_none(), "must have not distortion");
	dataset_group datag = datas.group(dataset_group_name);

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
	const cv::Vec3b background_color(0, 0, 0);
	cv::Size image_size = datag.image_size_with_border();
	cv::Mat_<cv::Vec3b> back_image(image_size, background_color);
	if(fpoints.view_idx) {
		view_index view_index = fpoints.view_idx;
		std::string image_filename = datas.view(view_index).image_filename();
		back_image = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
		image_size = back_image.size();
	}
	
	viewer view("Slopes Viewer", image_size.width, image_size.height+20+(has_measured_slopes ? 20 : 0));
	const real max_abs_angle = 30.0_deg;
	auto& x_slider = view.add_real_slider("X", 0.0, -max_abs_angle, +max_abs_angle);
	auto& y_slider = view.add_real_slider("Y", 0.0, -max_abs_angle, +max_abs_angle);
	auto& z_slider = view.add_real_slider("Z", 0.0, -max_abs_angle, +max_abs_angle);
	auto& measured_width_slider = view.add_int_slider("measured width", 0, 0, 400);
	auto& model_width_slider = view.add_int_slider("model width", 0, 200, 400);
	auto& exaggeration_slider = view.add_real_slider("exaggeration", 1.0, 1.0, 100.0);


	view.update_callback = [&]() {
		// parameters
		real x = x_slider.value(), y = y_slider.value(), z = z_slider.value();
		real exaggeration = exaggeration_slider.value();
			
		cv::Mat_<cv::Vec3b> viz_image(image_size);	
				
		// draw background
		back_image.copyTo(viz_image);

		// draw measured slopes
		if(has_measured_slopes && measured_width_slider.value() > 0)
			viz_image = visualize_feature_slopes(measured_fslopes, viz_image, measured_width_slider.value(), exaggeration, 2);

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
		mat33 R = to_rotation_matrix(x, y, z);
		for(const auto& kv : fpoints.points) {
			const std::string& feature_name = kv.first;
			const feature_point& fpoint = kv.second;
			feature_slope& fslope = model_fslopes.slopes[feature_name];
			fslope.horizontal = model_horizontal_slope(fpoint.position, intr.K, R);
			fslope.vertical = model_vertical_slope(fpoint.position, intr.K, R);
		}
		if(model_width_slider > 0)
			viz_image = visualize_feature_slopes(model_fslopes, viz_image, model_width_slider, exaggeration, 1);
		
		view.clear();
		view.draw(cv::Point(0, 20), viz_image);

		real herror = NAN;
		real verror = NAN;
		real error = NAN;
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
		std::string label = "x=" + std::to_string(x * deg_per_rad) + "    y=" + std::to_string(y * deg_per_rad) + "    z=" + std::to_string(z * deg_per_rad);
		view.draw_text(cv::Rect(10, 0, image_size.width-20, 20), label, viewer::left);
	
		if(has_measured_slopes) {
			label = "herror=" + std::to_string(herror) + "    verror=" + std::to_string(verror) + "    error=" + std::to_string(error);
			view.draw_text(cv::Rect(10, 20+image_size.height, image_size.width-20, 20), label, viewer::left);
		}
	};

	view.show_modal();
}
