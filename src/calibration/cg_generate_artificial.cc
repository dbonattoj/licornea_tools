#include <string>
#include <map>
#include <cmath>
#include <random>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/image_io.h"
#include "../lib/random_color.h"
#include "../lib/intrinsics.h"
#include "../lib/filesystem.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "rotation.json intrinsics.json out_datas_dir/ [features_count=100] [num_x=30] [num_y=10] [step_x=3.0] [step_y=3.0]");
	mat33 R = decode_mat(json_arg());
	intrinsics intr = intrinsics_arg();
	std::string out_datas_dir = out_filename_arg();
	int features_count = int_opt_arg(100);
	int num_x = int_opt_arg(30);
	int num_y = int_opt_arg(20);
	real step_x = real_opt_arg(3.0);
	real step_y = real_opt_arg(3.0);

	mat33 K = intr.K;
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);	
	
	int width = intr.width;
	int height = intr.height;
	
	std::cout << "generating " << features_count << " random 3D features" << std::endl;
	std::vector<vec3> features;
	{
		std::uniform_real_distribution<real> ix_dist(0, width);
		std::uniform_real_distribution<real> iy_dist(0, height);
		std::uniform_real_distribution<real> vz_dist(500.0, 3000.0);	
		std::mt19937 gen;

		for(int i = 0; i < features_count; ++i) {
			real ix = ix_dist(gen);
			real iy = iy_dist(gen);
			real vz = vz_dist(gen);
			
			real vx = (ix - cx)*vz/fx;
			real vy = (iy - cy)*vz/fy;
			
			vec3 v(vx, vy, vz);
			vec3 w = R.t() * v;
			
			features.push_back(w);
		}
	}
	
	
	std::cout << "creating directories" << std::endl;
	make_directory(out_datas_dir);
	make_directory(out_datas_dir + "/image");
	make_directory(out_datas_dir + "/depth");
	make_directory(out_datas_dir + "/warped_image");
	make_directory(out_datas_dir + "/warped_depth");


	std::cout << "generating dataset parameters" << std::endl;
	{
		json j_dataset = json::object();
		j_dataset["x_index_range"] = json::array({0, num_x-1});
		j_dataset["y_index_range"] = json::array({0, num_y-1});
		j_dataset["width"] = width;
		j_dataset["height"] = height;
		j_dataset["image_filename_format"] = "image/y{y}_x{x}.png";
		j_dataset["depth_filename_format"] = "depth/y{y}_x{x}.png";
		j_dataset["cameras_filename"] = "cameras.json";
		j_dataset["camera_name_format"] = "camera_y{y}_x{x}";
		j_dataset["warped"] = json::object();
		j_dataset["warped"]["image_filename_format"] = "warped_image/y{y}_x{x}.png";
		j_dataset["warped"]["depth_filename_format"] = "warped_depth/y{y}_x{x}.png";
		export_json_file(j_dataset, out_datas_dir + "/parameters.json");
	}
	
	
	std::cout << "generating correspondences by projecting features for each view" << std::endl;
	image_correspondences cors;
	view_index reference_view(num_x/2, num_y/2);
	std::map<view_index, vec3> view_camera_centers;
	for(int y = 0; y < num_y; ++y) for(int x = 0; x < num_x; ++x) {
		view_index idx(x, y);
				
		// camera center position
		real px = step_x*(x - num_x/2);
		real py = step_y*(y - num_y/2);
		vec3 p(px, py, 0.0);
		view_camera_centers[idx] = p;
		
		
		for(int feature = 0; feature < features_count; ++feature) {
			// feature image position in this view
			const vec3& w = features[feature];
			vec3 v = R*(w + p);
			vec3 i_ = K*v;
			vec2 i(i_[0]/i_[2], i_[1]/i_[2]);
			vec2 dist_i = distort_point(intr, i);
		
			// add image correspondence
			std::string feature_name = "feat" + std::to_string(feature);
			feature_point fpoint;
			fpoint.position = dist_i;
			fpoint.depth = v[2];
			fpoint.weight = 1.0;
			
			auto feature_it = cors.features.find(feature_name);
			if(feature_it == cors.features.end()) {
				image_correspondence_feature feature;
				feature.reference_view = reference_view;
				auto res = cors.features.emplace(feature_name, std::move(feature));
				feature_it = res.first;
			}
			
			feature_it->second.points[idx] = fpoint;
		}
	}
	
	
	std::cout << "saving correspondences" << std::endl;
	export_json_file(encode_image_correspondences(cors), out_datas_dir + "/cors.json");
	
	
	std::cout << "saving cameras" << std::endl;
	camera_array cams;
	for(const auto& kv : view_camera_centers) {
		view_index idx = kv.first;
		const vec3& p = kv.second;
		vec3 t = R * p;
			
		camera cam;
		cam.name = "camera_y" + std::to_string(idx.y) + "_x" + std::to_string(idx.x);
		cam.intrinsic = K;
		cam.rotation = R;
		cam.translation = t;
		cams.push_back(cam);
	}
	std::string cameras_filename = out_datas_dir + "/cameras.json";
	write_cameras_file(cameras_filename, cams);
	
	
	std::cout << "saving straight depths" << std::endl;
	{
		json j_straight_depths = json::object();
		for(int feature = 0; feature < features_count; ++feature) {
			std::string feature_name = "feat" + std::to_string(feature);
			const vec3& wp = features.at(feature);
			
			j_straight_depths[feature_name] = wp[2];
		}
		export_json_file(j_straight_depths, out_datas_dir + "/straight_depths.json");
	}
	
	
	std::cout << "drawing images and depth maps" << std::endl;
	const cv::Vec3b background_color(0, 0, 0);
	const real small_radius = 4.0;
	const real large_radius = 60.0;
	const real focal = (intr.fx() + intr.fy()) / 2.0;
	
	#pragma omp parallel for
	for(int y = 0; y < num_y; ++y) for(int x = 0; x < num_x; ++x) {
		view_index idx(x, y);
		std::cout << '.' << std::flush;

		// images
		cv::Mat_<cv::Vec3b> texture_image(height, width, background_color);		
		cv::Mat_<ushort> depth_image(height, width, ushort(0xffff));

		for(int feature = 0; feature < features_count; ++feature) {
			std::string feature_name = "feat" + std::to_string(feature);
			const feature_point& fpoint = cors.features.at(feature_name).points.at(idx);
			cv::Point pos = vec2_to_point(fpoint.position);
			real depth = fpoint.depth;

			cv::Mat_<cv::Vec3b> feature_texture_image(height, width, background_color);
			cv::Mat_<ushort> feature_depth_image(height, width, ushort(0xffff));
			cv::Mat_<uchar> feature_mask_image(height, width, uchar(0));
			
			int small_radius_pix = focal * small_radius / depth;
			int large_radius_pix = focal * large_radius / depth;
			
			cv::Vec3b col = random_color(feature);
			cv::Vec3b col_darker = 0.3 * col;
				
			cv::circle(feature_texture_image, pos, large_radius_pix, cv::Scalar(col_darker), -1);
			cv::circle(feature_texture_image, pos, small_radius_pix, cv::Scalar(col), -1);
			cv::circle(feature_depth_image, pos, large_radius_pix, depth, -1);
			cv::circle(feature_mask_image, pos, large_radius_pix, 255, -1);
			
			cv::Mat_<uchar> mask = feature_mask_image & (feature_depth_image < depth_image);
			feature_texture_image.copyTo(texture_image, mask);
			feature_depth_image.copyTo(depth_image, mask);
		}
		depth_image.setTo(ushort(0), (depth_image == 0xffff));
		
		// save images
		std::string texture_image_filename = out_datas_dir + "/image/y" + std::to_string(y) + "_x" + std::to_string(x) + ".png";
		std::string depth_image_filename = out_datas_dir + "/depth/y" + std::to_string(y) + "_x" + std::to_string(x) + ".png";
		save_texture(texture_image_filename, texture_image);
		save_depth(depth_image_filename, depth_image);
	}
	std::cout << std::endl;
		
	
	std::cout << "done" << std::endl;
}
