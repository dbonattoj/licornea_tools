#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include "../lib/filesystem.h"
#include "lib/cg/references_grid.h"
#include "lib/feature_points.h"
#include <string>
#include <format.h>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [refgrid.json] [out_fpoints_dir/] [dataset_group]");
	dataset datas = dataset_arg();
	std::string refgrid_filename = in_filename_opt_arg();
	std::string out_fpoints_dirname = out_dirname_opt_arg();
	std::string dataset_group_name = string_opt_arg("");

	dataset_group datag = datas.group(dataset_group_name);
	
	viewer view("Dataset Viewer", datag.image_size_with_border(), true);
	std::function<view_index()> get_shown_idx;
	references_grid grid;
	if(refgrid_filename.empty()) {
		const auto& slider_x = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
		auto& slider_y = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
		get_shown_idx = [&]() {
			return view_index(slider_x, slider_y);
		};
	} else {
		grid = decode_references_grid(import_json_file(refgrid_filename));
		auto& slider_col = view.add_int_slider("r_col", 0, 0, grid.cols()-1);
		auto& slider_row = view.add_int_slider("r_row", 0, 0, grid.rows()-1);
		get_shown_idx = [&]() {
			return grid.view(slider_col, slider_row);
		};
	}


	auto& min_distance_between_features = view.add_int_slider("dist", 60, 1, 200);
	auto& features_quality_level = view.add_real_slider("qual", 0.5, 0.0, 1.0);
	auto& wanted_features = view.add_int_slider("wanted", 100, 1, 300);
	auto& subdivisions = view.add_int_slider("subdiv", 2, 1, 4);

	auto choose_features = [&](const cv::Mat_<cv::Vec3b>& orig_img) {
		cv::Mat_<uchar> img;
		cv::cvtColor(orig_img, img, CV_BGR2GRAY);

		int width = img.cols, height = img.rows;
			
		const int pieces_count = subdivisions * subdivisions;
		auto get_piece_mask = [&](int piece) {
			int w = width/subdivisions;
			int h = height/subdivisions;
			int x = (piece % subdivisions) * w;
			int y = (piece / subdivisions) * h;
			
			cv::Mat_<uchar> mask(height, width);
			mask.setTo(0);
			mask(cv::Rect(x, y, w, h)).setTo(255);
			return mask;
		};
	
		std::vector<std::vector<cv::Point2f>> piece_positions(pieces_count);
		#pragma omp parallel for
		for(int piece = 0; piece < pieces_count; ++piece) {
			cv::Mat_<uchar> piece_mask = get_piece_mask(piece);
			cv::goodFeaturesToTrack(img, piece_positions[piece], wanted_features, features_quality_level, min_distance_between_features, piece_mask);
		}
		
		std::vector<cv::Point2f> positions;
		int i = 0;
		while(positions.size() < wanted_features) {
			bool none_left = true;
			for(const std::vector<cv::Point2f>& piece_pos : piece_positions) {
				if(piece_pos.size() <= i) continue;
				positions.push_back(piece_pos[i]);
				none_left = false;	
			}
			if(none_left) break;
			++i;
		}
		
		return positions;
	};

	view.update_callback = [&]() {
		view_index idx = get_shown_idx();
		if(! datas.valid(idx)) return;		
		
		std::string filename = datag.view(idx).image_filename();
		try {
			cv::Mat_<cv::Vec3b> img = load_texture(filename);
			std::vector<cv::Point2f> features = choose_features(img);
			for(cv::Point2f pt : features) {
				cv_aa_circle(img, pt, 7, cv::Scalar(cv::Vec3b(255, 255, 255)), 3);
				cv_aa_circle(img, pt, 5, cv::Scalar(cv::Vec3b(0, 0, 255)), 2);
			}
			view.draw(img);
		} catch(const std::runtime_error&) {
			std::cout << "could not load " << filename << std::endl; 
			view.clear();
		}
	};
	
	auto unique_feature_name = [](int number) {
		return fmt::format("feat{:04d}", number);
	};

	auto export_fpoints = [&](view_index idx, int global_index_base = 0) {
		int number = 0;
		std::string img_filename = datag.view(idx).image_filename();
		cv::Mat_<cv::Vec3b> img = load_texture(img_filename);
		std::vector<cv::Point2f> features = choose_features(img);
		feature_points fpoints;
		fpoints.view_idx = idx;
		for(const cv::Point2f& pt : features) {
			std::string feature_name = unique_feature_name(global_index_base + number);
			number++;
			feature_point fpoint;
			fpoint.position = point2f_to_vec2(pt);
			fpoints.points[feature_name] = fpoint;
		}
		
		std::string out_filename = "fpoints_" + encode_view_index(idx) + ".json";
		out_filename = filename_append(out_fpoints_dirname, out_filename);
		export_json_file(encode_feature_points(fpoints), out_filename);
	};
	
	view.key_callback = [&](int keycode) {
		if(keycode != enter_keycode) return;
		
		if(grid.is_valid()) {
			std::cout << "saving feature points for all reference views" << std::endl;
			int global_index_base = 0;
			for(std::ptrdiff_t col = 0; col < grid.cols(); ++col)
			for(std::ptrdiff_t row = 0; row < grid.rows(); ++row) {
				export_fpoints(grid.view(col, row), global_index_base);
				global_index_base += 1000;	
			}
				
		} else {
			std::cout << "saving feature points for this view" << std::endl;
			export_fpoints(get_shown_idx());
		}
	};

	view.show_modal();
}
