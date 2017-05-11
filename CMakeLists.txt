cmake_minimum_required(VERSION 3.1)
project(licornea_tools CXX)

set(CMAKE_CXX_STANDARD 14)


macro(install_pylib_link section)
	install(
		CODE "execute_process( \
			WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX} \
			COMMAND ${CMAKE_COMMAND} -E create_symlink \
			../lib/pylib \
			${section}/pylib \
		)"
	)
endmacro(install_pylib_link)


find_package(OpenCV REQUIRED)

find_package(OpenMP)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(SYSTEM src/external/include)
include_directories(SYSTEM src/external/fmt)
include_directories(SYSTEM src/external/mf)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBRARIES})

add_compile_options(-Wall -Wextra -Wno-sign-compare)

file(GLOB_RECURSE COMMON_LIB_SRC "src/lib/*.cc")
add_library(common_lib SHARED ${COMMON_LIB_SRC} "src/external/fmt/format.cc")

target_link_libraries(common_lib ${OpenCV_LIBRARIES})

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


install(
	TARGETS common_lib
	DESTINATION "lib/"
)
install(
	DIRECTORY "src/lib/pylib"
	DESTINATION "./lib"
	PATTERN "*.py"
)

# Include Freenect2, if WITH_LIBFREENECT2 is set
set(WITH_LIBFREENECT2 FALSE CACHE BOOL "Use libfreenect2")
if(WITH_LIBFREENECT2)
	include_directories(SYSTEM src/external/freenect2/include)
	link_directories(src/external/freenect2/lib)
	add_compile_options(-DLICORNEA_WITH_LIBFREENECT2)
	install(
		DIRECTORY "src/external/freenect2/lib"
		DESTINATION "./"
		PATTERN "*.so"
		PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE GROUP_EXECUTE
	)
endif()


# Calibration
file(GLOB_RECURSE CALIBRATION_LIB_SRC "src/calibration/lib/*.cc")
add_library(calibration_lib STATIC ${CALIBRATION_LIB_SRC})

add_executable(find_chessboard "src/calibration/find_chessboard.cc")
target_link_libraries(find_chessboard common_lib calibration_lib)

add_executable(calibrate_distortion "src/calibration/calibrate_distortion.cc")
target_link_libraries(calibrate_distortion common_lib calibration_lib)

add_executable(undistort "src/calibration/undistort.cc")
target_link_libraries(undistort common_lib calibration_lib)

add_executable(calibrate_extrinsic "src/calibration/calibrate_extrinsic.cc")
target_link_libraries(calibrate_extrinsic common_lib calibration_lib)

add_executable(optical_flow "src/calibration/optical_flow.cc")
target_link_libraries(optical_flow common_lib calibration_lib)

add_executable(optical_flow_2d "src/calibration/optical_flow_2d.cc")
target_link_libraries(optical_flow_2d common_lib calibration_lib)

add_executable(visualize_image_correspondences "src/calibration/visualize_image_correspondences.cc")
target_link_libraries(visualize_image_correspondences common_lib calibration_lib)

add_executable(feature_depth_estimate "src/calibration/feature_depth_estimate.cc")
target_link_libraries(feature_depth_estimate common_lib calibration_lib)

add_executable(cg_optical_flow_slopes "src/calibration/cg_optical_flow_slopes.cc")
target_link_libraries(cg_optical_flow_slopes common_lib calibration_lib)

add_executable(cg_visualize_optical_flow_slopes "src/calibration/cg_visualize_optical_flow_slopes.cc")
target_link_libraries(cg_visualize_optical_flow_slopes common_lib calibration_lib)

add_executable(cg_generate_artificial "src/calibration/cg_generate_artificial.cc")
target_link_libraries(cg_generate_artificial common_lib calibration_lib)


install(
	TARGETS
		find_chessboard
		calibrate_distortion
		undistort 
		calibrate_extrinsic
		optical_flow
		optical_flow_2d
		visualize_image_correspondences
		feature_depth_estimate
		cg_optical_flow_slopes
		cg_visualize_optical_flow_slopes
		cg_generate_artificial
	DESTINATION calibration
)
install_pylib_link(calibration)


# Camera
file(GLOB_RECURSE CAMERA_LIB_SRC "src/camera/lib/*.cc")
add_library(camera_lib STATIC ${CAMERA_LIB_SRC})

add_executable(export_mpeg "src/camera/export_mpeg.cc")
target_link_libraries(export_mpeg common_lib camera_lib)

add_executable(import_mpeg "src/camera/import_mpeg.cc")
target_link_libraries(import_mpeg common_lib camera_lib)

add_executable(transform "src/camera/transform.cc")
target_link_libraries(transform common_lib camera_lib)

add_executable(visualize "src/camera/visualize.cc")
target_link_libraries(visualize common_lib camera_lib)

install(
	TARGETS
		export_mpeg
		import_mpeg
		transform
		visualize
	DESTINATION camera
)
install(
	PROGRAMS
		"src/camera/import_xml.py"
		"src/camera/import_matlab.py"
	DESTINATION camera
)
install_pylib_link(camera)


# Kinect
file(GLOB_RECURSE KINECT_LIB_SRC "src/kinect/lib/*.cc")
add_library(kinect_lib STATIC ${KINECT_LIB_SRC})

add_executable(depth_point_cloud "src/kinect/depth_point_cloud.cc")
target_link_libraries(depth_point_cloud common_lib kinect_lib)

add_executable(depth_reprojection "src/kinect/depth_reprojection.cc")
target_link_libraries(depth_reprojection common_lib kinect_lib)

install(
	TARGETS
		depth_point_cloud
		depth_reprojection
	DESTINATION kinect
)
install(
	PROGRAMS
		"src/kinect/import_raw_data.py"
	DESTINATION kinect
)
install_pylib_link(kinect)

if(WITH_LIBFREENECT2)
	add_executable(fetch_intrinsics "src/kinect/fetch_intrinsics.cc")
	target_link_libraries(fetch_intrinsics common_lib kinect_lib freenect2)

	add_executable(live_view "src/kinect/live_view.cc")
	target_link_libraries(live_view common_lib kinect_lib freenect2)

	add_executable(live_tetrachrom "src/kinect/live_tetrachrom.cc")
	target_link_libraries(live_tetrachrom common_lib kinect_lib freenect2)

	add_executable(live_parallel_checkerboard "src/kinect/live_parallel_checkerboard.cc")
	target_link_libraries(live_parallel_checkerboard common_lib kinect_lib freenect2)

	add_executable(live_parallel_background "src/kinect/live_parallel_background.cc")
	target_link_libraries(live_parallel_background common_lib kinect_lib freenect2)

	install(
		TARGETS
			fetch_intrinsics
			live_view
			live_tetrachrom
			live_parallel_checkerboard
			live_parallel_background
		DESTINATION kinect
	)
endif()


# Dataset
install(
	PROGRAMS
		"src/dataset/slice.py"
		"src/dataset/flip.py"
	DESTINATION dataset
)
install_pylib_link(dataset)


# VSRS
add_executable(vsrs_disparity "src/vsrs/vsrs_disparity.cc")
target_link_libraries(vsrs_disparity common_lib kinect_lib)

install(
	TARGETS
		vsrs_disparity
	DESTINATION vsrs
)
install(
	PROGRAMS
		"src/vsrs/export_for_vsrs.py"
		"src/vsrs/make_vsrs_config.py"
		"src/vsrs/run_vsrs_experiments.py"
		"src/vsrs/run_vsrs.py"
	DESTINATION vsrs
)
install_pylib_link(vsrs)




# Misc
add_executable(psnr "src/misc/psnr.cc")
target_link_libraries(psnr common_lib)

add_executable(view_depth "src/misc/view_depth.cc")
target_link_libraries(view_depth common_lib)

add_executable(copy_json "src/misc/copy_json.cc")
target_link_libraries(copy_json common_lib)

add_executable(cam_rotation "src/misc/cam_rotation.cc")
target_link_libraries(cam_rotation common_lib)


install(
	TARGETS
		psnr
		view_depth
		copy_json
		cam_rotation
	DESTINATION misc
)
install(
	PROGRAMS
		"src/misc/list_skip_n_experiments.py"
		"src/misc/yuv2png.sh"
	DESTINATION misc
)
install_pylib_link(misc)
