#include <string>
#include <map>
#include <cmath>
#include <random>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/image_io.h"
#include "../lib/random_color.h"
#include "../lib/intrinsics.h"
#include "../lib/filesystem.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"rotation.json intrinsics.json out_datas_dir/ [features_count=100] [num_x=30] [num_y=10] [step_x=1.0] [step_y=1.0]");
	mat33 R = decode_mat(json_arg());
	intrinsics intr = intrinsics_arg();
	std::string out_datas_dir = out_filename_arg();
	int features_count = int_opt_arg(100);
	int num_x = int_opt_arg(30);
	int num_y = int_opt_arg(10);
	real step_x = real_opt_arg(1.0);
	real step_y = real_opt_arg(1.0);

	mat33 K = intr.K;
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);	
	
	int width = intr.width;
	int height = intr.height;
	
	std::cout << "generating " << features_count << " random 3D features" << std::endl;
	std::vector<vec3> features;
	{
		std::uniform_real_distribution<real> ix_dist(0, width);
		std::uniform_real_distribution<real> iy_dist(0, height);
		std::uniform_real_distribution<real> vz_dist(500.0, 2000.0);	
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
	make_directory(out_datas_dir + "/rectified_image");
	make_directory(out_datas_dir + "/rectified_depth");


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
		j_dataset["rectified"] = json::object();
		j_dataset["rectified"]["image_filename_format"] = "rectified_image/y{y}_x{x}.png";
		j_dataset["rectified"]["depth_filename_format"] = "rectified_depth/y{y}_x{x}.png";
		export_json_file(j_dataset, out_datas_dir + "/parameters.json");
	}
	
	
	std::cout << "generating correspondences by projecting features for each view" << std::endl;
	image_correspondences cors;
	cors.reference = view_index(num_x/2, num_y/2);
	std::map<view_index, vec3> view_camera_centers;
	std::map<std::string, real> straight_depths;
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
			cors.features[feature_name].points[idx] = dist_i;
			cors.features[feature_name].depth = v[2];
			
			straight_depths[feature_name] = v[2];
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
		for(const auto& kv : straight_depths)
			j_straight_depths[kv.first] = kv.second;
		export_json_file(j_straight_depths, out_datas_dir + "/straight_depths.json");
	}
	
	
	std::cout << "drawing images and depth maps" << std::endl;
	#pragma omp parallel for
	for(int y = 0; y < num_y; ++y) for(int x = 0; x < num_x; ++x) {
		view_index idx(x, y);
		std::cout << '.' << std::flush;

		// images
		cv::Mat_<cv::Vec3b> texture_image(height, width);
		texture_image.setTo(cv::Vec3b(0, 0, 0));
		
		cv::Mat_<ushort> depth_image(height, width);
		depth_image.setTo(0);

		const int blob_radius = 4;

		for(int feature = 0; feature < features_count; ++feature) {
			std::string feature_name = "feat" + std::to_string(feature);
			vec2 dist_i = cors.features.at(feature_name).points.at(idx);
			real depth = cors.features.at(feature_name).depth;
			
			cv::Vec3b col = random_color(feature);

			// draw blob in texture image
			cv::circle(texture_image, vec2_to_point(dist_i), blob_radius, cv::Scalar(col), -1);
			
			// draw blob in depth image
			cv::circle(depth_image, vec2_to_point(dist_i), blob_radius, depth, -1);
		}
		
		// save images
		std::string texture_image_filename = out_datas_dir + "/image/y" + std::to_string(y) + "_x" + std::to_string(x) + ".png";
		std::string depth_image_filename = out_datas_dir + "/depth/y" + std::to_string(y) + "_x" + std::to_string(x) + ".png";
		save_texture(texture_image_filename, texture_image);
		save_depth(depth_image_filename, depth_image);
	}
	std::cout << std::endl;
		
	
	std::cout << "done" << std::endl;
}
