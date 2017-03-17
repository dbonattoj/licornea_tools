cmake_minimum_required(VERSION 3.1)
project(licornea_tools CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

include_directories(SYSTEM external/include)
include_directories(SYSTEM external/fmt)
include_directories(SYSTEM external/mf)
include_directories(SYSTEM external/freenect2/include)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
link_directories(external/freenect2/lib)
link_libraries(${OpenCV_LIBRARIES})

add_compile_options(-Wall -Wextra -Wno-sign-compare)

file(GLOB_RECURSE COMMON_LIB_SRC "lib/*.cc")

add_library(common_lib SHARED ${COMMON_LIB_SRC})
target_link_libraries(common_lib ${OpenCV_LIBRARIES})

add_library(fmt_lib STATIC "external/fmt/format.cc")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

install(
	DIRECTORY "external/freenect2/lib"
	DESTINATION "./"
	PATTERN "*.so"
	PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE GROUP_EXECUTE
)
install(
	TARGETS common_lib
	DESTINATION "lib/"
)

# Calibration
file(GLOB_RECURSE CALIBRATION_LIB_SRC "calibration/lib/*.cc")
add_library(calibration_lib STATIC ${CALIBRATION_LIB_SRC})

add_executable(find_chessboard "calibration/find_chessboard.cc")
target_link_libraries(find_chessboard common_lib calibration_lib)

add_executable(calibrate_distortion "calibration/calibrate_distortion.cc")
target_link_libraries(calibrate_distortion common_lib calibration_lib)

add_executable(undistort "calibration/undistort.cc")
target_link_libraries(undistort common_lib calibration_lib)

add_executable(calibrate_extrinsic "calibration/calibrate_extrinsic.cc")
target_link_libraries(calibrate_extrinsic common_lib calibration_lib)

install(
	TARGETS find_chessboard calibrate_distortion undistort calibrate_extrinsic
	DESTINATION calibration
)


# Camera
add_executable(export_mpeg "camera/export_mpeg.cc")
target_link_libraries(export_mpeg common_lib)

add_executable(import_mpeg "camera/import_mpeg.cc")
target_link_libraries(import_mpeg common_lib)

add_executable(transform "camera/transform.cc")
target_link_libraries(transform common_lib fmt_lib)

add_executable(visualize "camera/visualize.cc")
target_link_libraries(visualize common_lib)

install(
	TARGETS export_mpeg import_mpeg transform visualize
	DESTINATION camera
)
install(
	PROGRAMS
		"camera/import_xml.py"
		"camera/make_regular_row.py"
	DESTINATION camera
)


# Kinect
file(GLOB_RECURSE KINECT_LIB_SRC "kinect/lib/*.cc")
add_library(kinect_lib STATIC ${KINECT_LIB_SRC})



add_executable(fetch_intrinsics "kinect/fetch_intrinsics.cc")
target_link_libraries(fetch_intrinsics common_lib kinect_lib freenect2)

add_executable(color_point_cloud "kinect/color_point_cloud.cc")
target_link_libraries(color_point_cloud common_lib kinect_lib freenect2)

add_executable(depth_point_cloud "kinect/depth_point_cloud.cc")
target_link_libraries(depth_point_cloud common_lib kinect_lib freenect2)

add_executable(depth_reprojection "kinect/depth_reprojection.cc")
target_link_libraries(depth_reprojection common_lib kinect_lib freenect2)

add_executable(texture_reprojection "kinect/texture_reprojection.cc")
target_link_libraries(texture_reprojection common_lib kinect_lib freenect2)

add_executable(point_cloud_reprojection "kinect/point_cloud_reprojection.cc")
target_link_libraries(point_cloud_reprojection common_lib kinect_lib freenect2)

add_executable(vsrs_disparity "kinect/vsrs_disparity.cc")
target_link_libraries(vsrs_disparity common_lib kinect_lib)

install(
	TARGETS fetch_intrinsics color_point_cloud depth_point_cloud depth_reprojection texture_reprojection point_cloud_reprojection vsrs_disparity
	DESTINATION kinect
)
install(
	PROGRAMS
		"kinect/make_vsrs_data.py"
		"kinect/depth_cross_reprojection.py"
		"kinect/collect_col.py"
		"kinect/multiprojection_window.py"
	DESTINATION kinect
)



# Misc
add_executable(psnr "misc/psnr.cc")
target_link_libraries(psnr common_lib)

add_executable(view_depth "misc/view_depth.cc")
target_link_libraries(view_depth common_lib)


install(
	TARGETS psnr view_depth
	DESTINATION misc
)
install(
	PROGRAMS
		"misc/list_skip_n_experiments.py"
		"misc/make_vsrs_config.py"
		"misc/run_vsrs_experiments.py"
		"misc/run_vsrs.py"
		"misc/yuv2png.sh"
	DESTINATION misc
)
