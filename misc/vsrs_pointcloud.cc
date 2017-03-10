#include <mf/common.h>
#include <mf/nd.h>
#include <mf/io/image_import.h>
#include <mf/geometry/depth_projection_parameters.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/camera/depth_image_mapping.h>
#include <mf/io/rs_camera_array.h>
#include <mf/point_cloud/point.h>
#include <mf/io/ply_exporter.h>
#include <mf/io/yuv_importer.h>
#include <mf/io/rs_config_reader.h>
#include <mf/color.h>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <iostream>

using namespace mf;

using ushort = std::uint16_t;
using uchar = std::uint8_t;

ndarray<2, rgb_color> import_texture(const std::string& yuv_filename, const ndsize<2>& shape) {
	yuv_importer imp(yuv_filename, shape, 420);
	ndarray<2, ycbcr_color> yuv_array(shape);
	ndarray<2, rgb_color> rgb_array(shape);
	imp.read_frame(yuv_array.view());
	std::transform(yuv_array.begin(), yuv_array.end(), rgb_array.begin(), color_convert<rgb_color, ycbcr_color>);
	return rgb_array;
}

ndarray<2, uchar> import_disparity(const std::string& yuv_filename, const ndsize<2>& shape) {
	yuv_importer imp(yuv_filename, shape, 420);
	ndarray<2, ycbcr_color> yuv_array(shape);
	ndarray<2, uchar> int_array(shape);
	imp.read_frame(yuv_array.view());
	std::transform(yuv_array.begin(), yuv_array.end(), int_array.begin(), [](const ycbcr_color& col) { return col.y; });
	return int_array;
}


ndarray<1, point_xyzrgb> make_point_cloud(const rs_config_reader& config, bool left) {
	real z_near, z_far;
	std::string texture_yuv, disparity_yuv, input_camera_name;
	if(left) {
		z_near = config.get_real("LeftNearestDepthValue");
		z_far = config.get_real("LeftFarthestDepthValue");
		input_camera_name = config.get_string("LeftCameraName");
		texture_yuv = config.get_string("LeftViewImageName");
		disparity_yuv = config.get_string("LeftDepthMapName");
	} else {
		z_near = config.get_real("RightNearestDepthValue");
		z_far = config.get_real("RightFarthestDepthValue");
		input_camera_name = config.get_string("RightCameraName");
		texture_yuv = config.get_string("RightViewImageName");
		disparity_yuv = config.get_string("RightDepthMapName");
	}
	
	std::cout << "reading camera array" << std::endl;
	auto proj = depth_projection_parameters::unsigned_normalized_disparity(z_near, z_far);
	proj.d_near = 0xff; proj.d_far = 0x00; //proj.flip_z = true;
	rs_camera_array cams(config.get_string("CameraParameterFile"), proj);
	
	auto cam = cams[input_camera_name];
	
	int width = config.get_int("SourceWidth");
	int height = config.get_int("SourceHeight");
	auto shape = make_ndsize(height, width);
	
	std::cout << "importing texture" << std::endl;
	ndarray<2, rgb_color> texture = import_texture(texture_yuv, shape);

	std::cout << "importing disparity" << std::endl;
	ndarray<2, uchar> disparity = import_disparity(disparity_yuv, shape);
	
	ndarray<1, point_xyzrgb> pc(make_ndsize(shape.product()));
	std::ptrdiff_t i = 0;
	
	std::cout << "projecting into point cloud" << std::endl;
	for(ndptrdiff<2> coord : make_ndspan(shape)) {
		uchar& d = disparity.at(coord);
		const rgb_color& col = texture.at(coord);
		if(d == 0) continue;
		point_xyzrgb& pt = pc[i++];
		
		pt.position() = cam.point(Eigen_vec3(coord[1], coord[0], d));
		pt.color() = col;
	}

	return pc;
}


int main(int argc, const char* argv[]) {
	if(argc <= 3) {
		std::cout << "usage: " << argv[0] << " vsrs_config.txt pointcloud.ply left/right/both" << std::endl;
		return EXIT_FAILURE;
	}
	const char* vsrs_config_filename = argv[1];
	const char* pointcloud_filename = argv[2];
	std::string mode = argv[3];
	
	std::cout << "reading config" << std::endl;
	rs_config_reader config(vsrs_config_filename);
	
	if(mode == "left") {
		std::cout << "making left point cloud" << std::endl;
		auto pc = make_point_cloud(config, true);
		
		std::cout << "exporting ply" << std::endl;
		ply_exporter ply(pointcloud_filename);
		ply.write(pc.view());

	} else if(mode == "right") {
		std::cout << "making right point cloud" << std::endl;
		auto pc = make_point_cloud(config, false);

		std::cout << "exporting ply" << std::endl;
		ply_exporter ply(pointcloud_filename);
		ply.write(pc.view());

		
	} else if(mode == "both") {
		std::cout << "making left point cloud" << std::endl;
		auto pc1 = make_point_cloud(config, true);
		
		std::cout << "making right point cloud" << std::endl;
		auto pc2 = make_point_cloud(config, false);
		
		std::cout << "merging point clouds" << std::endl;
		ndarray<1, point_xyzrgb> pc(pc1.shape() + pc2.shape());
		pc(0, pc1.size()) = pc1.view();
		pc(pc2.size(), -1) = pc2.view();
		
		std::cout << "exporting ply" << std::endl;
		ply_exporter ply(pointcloud_filename);
		ply.write(pc.view());
	
	}
}
