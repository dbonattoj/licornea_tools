#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/assert.h"
#include "../lib/opencv.h"
#include "../lib/image_io.h"
#include "../lib/filesystem.h"

using namespace tlz;

const bool verbose = false;


std::vector<vec3> checkerboard_world_corners(int cols, int rows, real square_width) {
	std::vector<vec3> world_corners(cols * rows);
	for(int row = 0, idx = 0; row < rows; ++row) for(int col = 0; col < cols; ++col, ++idx)
		world_corners[idx] = vec3((col - cols/2)*square_width, (row - rows/2)*square_width, 0.0);
	return world_corners;
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cols rows square_width intr.json out_cameras.json [dataset_group]");
	dataset datas = dataset_arg();
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	intrinsics intr = intrinsics_arg();
	std::string out_cameras_filename = out_filename_arg();
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);
	
	camera_array cameras;
	
	int i = 0;
	for(view_index idx : datas.indices()) {
		if(i++ % 100 == 0) std::cout << i << " of " << datas.indices().size() << std::endl;
		else std::cout << '.' << std::flush;
		
		if(idx.y != datas.y_min() && idx.x != datas.x_min()) continue;
		
		// load image
		dataset_view view = datag.view(idx);
		std::string image_filename = view.image_filename();
		std::string camera_name = view.camera_name();
		if(! file_exists(image_filename)) {
			std::cout << idx << ": no image file" << std::endl;
			continue; 
		}
		
		cv::Mat_<cv::Vec3b> img;
		try {
			img = load_texture(image_filename);
		} catch(...) {
			std::cout << idx << ": could not load image" << std::endl;
			continue;
		}
		
		
		// find checkerboard
		std::vector<cv::Point2f> corners;
		int flags = cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH;
		bool found = cv::findChessboardCorners(img, cv::Size(cols, rows), corners, flags);
		if(!found || corners.size() != cols*rows) {
			std::cout << idx << ": no checkerboard detected" << std::endl;
			continue;
		}
		
		
		// improve corners
		cv::TermCriteria term(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 100, DBL_EPSILON);
		cv::Mat img_mono;
		cv::cvtColor(img, img_mono, CV_BGR2GRAY);
		cv::cornerSubPix(img_mono, corners, cv::Size(11, 11), cv::Size(-1, -1), term);	

		
		// estimate camera pose
		std::vector<vec2> image_points = point2f_to_vec2(corners);
		std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

		vec3 rotation_vec, translation;
		mat33 rotation;	
		bool ret = cv::solvePnP(
			object_points,
			image_points,
			intr.K,
			intr.distortion.cv_coeffs(),
			rotation_vec,
			translation,
			false
		);
		if(! ret) {
			std::cout << idx << ": solvePnP failed" << std::endl;
			continue;
		}
		cv::Rodrigues(rotation_vec, rotation);
		
		
		// add camera
		camera cam;
		cam.name = camera_name;
		cam.intrinsic = intr.K;
		cam.rotation = rotation;
		cam.translation = translation;
		cameras.push_back(cam);
	}
	std::cout << std::endl;
	

	std::cout << "saving cameras" << std::endl;
	export_cameras_file(cameras, out_cameras_filename);
}
