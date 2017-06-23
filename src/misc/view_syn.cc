#include "../lib/common.h"
#include "../lib/assert.h"
#include "../lib/args.h"
#include "../lib/dataset.h"
#include "../lib/camera.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/viewer.h"
#include <iostream>

using namespace tlz;

cv::Mat_<cv::Vec3b> view_synthesis(
	const cv::Mat_<cv::Vec3b>& ref_img,
	const cv::Mat_<cv::Vec3b>& tg_img, const cv::Mat_<ushort>& tg_depth,
	const camera& ref_cam, const camera& tg_cam,
	real z_near, real z_far,
	real opacity, real darken_background
) {
	cv::Size sz = ref_img.size();
	Assert(tg_img.size() == sz);
	Assert(tg_depth.size() == sz);
	
	
	real z_diff = z_far - z_near;
	
	real alpha = -z_near / z_diff;
	real beta = z_near*z_far / z_diff;
		
	mat44 P_ref = {
		ref_cam.intrinsic(0,0), 0.0, ref_cam.intrinsic(0,2), 0.0,
		0.0, ref_cam.intrinsic(1,1), ref_cam.intrinsic(1,2), 0.0,
		0.0, 0.0, alpha, beta,
		0.0, 0.0, 1.0, 0.0
	};
	mat44 P_tg = {
		tg_cam.intrinsic(0,0), 0.0, tg_cam.intrinsic(0,2), 0.0,
		0.0, tg_cam.intrinsic(1,1), tg_cam.intrinsic(1,2), 0.0,
		0.0, 0.0, alpha, beta,
		0.0, 0.0, 1.0, 0.0
	};
	
	mat44 H = P_ref * ref_cam.extrinsic() * tg_cam.extrinsic_inv() * P_tg.inv();
	
	cv::Mat_<cv::Vec3b> out_img(sz);
	ref_img.copyTo(out_img);
	out_img *= darken_background;

	cv::Mat_<real> out_img_zbuffer(sz);
	out_img_zbuffer.setTo(0.0);
	
	#pragma omp parallel shared(out_img, ref_img, tg_img, tg_depth)
	{
		cv::Mat_<real> local_out_img_zbuffer(sz);
		local_out_img_zbuffer.setTo(0.0);

		cv::Mat_<cv::Vec3b> local_out_img(sz);

		#pragma omp for
		for(int x = 0; x < sz.width; ++x) for(int y = 0; y < sz.height; ++y) {			
			cv::Vec3b tg_col = tg_img(y, x);
			ushort d_int = tg_depth(y, x);
			if(d_int == 0) continue;
			real d = d_int;
			
			real disp = alpha + beta/d;
			
			vec4 tg_pos_h(x, y, disp, 1.0);
			vec4 ref_pos_h = H * tg_pos_h;
			
			vec2 ref_pos(ref_pos_h[0]/ref_pos_h[3], ref_pos_h[1]/ref_pos_h[3]);
			real ref_disp = ref_pos_h[2]/ref_pos_h[3];
			int ref_x = ref_pos[0], ref_y = ref_pos[1];
			if(ref_x < 0 || ref_x >= sz.width || ref_y < 0 || ref_y >= sz.height) continue;
			
			real& out_z = local_out_img_zbuffer(ref_y, ref_x);
			cv::Vec3b& out_col = local_out_img(ref_y, ref_x);
			
			cv::Vec3b ref_col = ref_img(ref_y, ref_x);		
			if(out_z < ref_disp) {
				out_z = ref_disp;
				out_col = (1-opacity)*ref_col + opacity*tg_col;
			}
		}

		#pragma omp critical
		{
			cv::Mat_<uchar> mask = (local_out_img_zbuffer > out_img_zbuffer);
			local_out_img.copyTo(out_img, mask);
			local_out_img_zbuffer.copyTo(out_img_zbuffer, mask);
		}
	}

	
	return out_img;
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cameras.json [dataset_group]");
	dataset datas = dataset_arg();
	camera_array cams = cameras_arg();
	std::string dataset_group_name = string_opt_arg();
	
	auto cams_map = cameras_map(cams);
	
	dataset_group datag = datas.group(dataset_group_name);
	
	viewer view("View Synthesis", datag.image_size_with_border(), true);
	auto& slider_ref_x = view.add_int_slider("ref X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& slider_ref_y = view.add_int_slider("ref Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
	auto& slider_tg_x = view.add_int_slider("tg X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& slider_tg_y = view.add_int_slider("tg Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
	auto& slider_opacity = view.add_real_slider("opacity", 0.5, 0.0, 1.0);
	auto& slider_darken_background = view.add_real_slider("darken", 0.3, 0.0, 1.0);

	real z_near = 600.0;
	real z_far = 1600.0;

	view.update_callback = [&]() {
		view.clear();
		try {
			view_index ref_idx(slider_ref_x, slider_ref_y);
			view_index tg_idx(slider_tg_x, slider_tg_y);
			if(! datas.valid(ref_idx) || ! datas.valid(tg_idx)) return;		
			
			cv::Mat_<cv::Vec3b> ref_image = load_texture(datag.view(ref_idx).image_filename());
			cv::Mat_<cv::Vec3b> tg_image = load_texture(datag.view(tg_idx).image_filename());
			cv::Mat_<ushort> tg_depth = load_depth(datag.view(tg_idx).depth_filename());
			camera ref_cam = cams_map.at(datas.view(ref_idx).camera_name());
			camera tg_cam = cams_map.at(datas.view(tg_idx).camera_name());
			
			cv::Mat_<cv::Vec3b> out_image = view_synthesis(
				ref_image, tg_image, tg_depth, ref_cam, tg_cam, z_near, z_far, slider_opacity, 1.0-slider_darken_background);
		
			view.draw(cv::Point(0,0), out_image);
		} catch(const std::runtime_error&) { }
	};

	view.show_modal();
}
