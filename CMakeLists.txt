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
if(WITH_LIB${FREENECT2_LIBRARY})
	include_directories(SYSTEM src/external/freenect2/include)
	find_library(FREENECT2_LIBRARY NAMES freenect2 libfreenect2 PATHS ${src/external/freenect2/lib})
	add_compile_options(-DLICORNEA_WITH_LIB${FREENECT2_LIBRARY})
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

add_executable(undistort "src/calibration/undistort.cc")
target_link_libraries(undistort common_lib calibration_lib)

add_executable(calibrate_intrinsics "src/calibration/calibrate_intrinsics.cc")
target_link_libraries(calibrate_intrinsics common_lib calibration_lib)

add_executable(view_feature_points "src/calibration/view_feature_points.cc")
target_link_libraries(view_feature_points common_lib calibration_lib)

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

add_executable(cg_predict_optical_flow_slopes "src/calibration/cg_predict_optical_flow_slopes.cc")
target_link_libraries(cg_predict_optical_flow_slopes common_lib calibration_lib)

add_executable(cg_estimate_rotation "src/calibration/cg_estimate_rotation.cc")
target_link_libraries(cg_estimate_rotation common_lib calibration_lib)

install(
	TARGETS
		calibrate_intrinsics
		view_feature_points
		undistort 
		optical_flow
		optical_flow_2d
		visualize_image_correspondences
		feature_depth_estimate
		cg_optical_flow_slopes
		cg_visualize_optical_flow_slopes
		cg_generate_artificial
		cg_predict_optical_flow_slopes
		cg_estimate_rotation
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

add_executable(depth_reprojection "src/kinect/depth_reprojection.cc")
target_link_libraries(depth_reprojection common_lib kinect_lib)

add_executable(internal_ir_intrinsics "src/kinect/internal_ir_intrinsics.cc")
target_link_libraries(internal_ir_intrinsics common_lib kinect_lib)

add_executable(calibrate_color_ir_reprojection "src/kinect/calibrate_color_ir_reprojection.cc")
target_link_libraries(calibrate_color_ir_reprojection common_lib kinect_lib)

add_executable(checkerboard_depth_stat "src/kinect/checkerboard_depth_stat.cc")
target_link_libraries(checkerboard_depth_stat common_lib kinect_lib ${FREENECT2_LIBRARY})

install(
	TARGETS
		depth_reprojection
		internal_ir_intrinsics
		calibrate_color_ir_reprojection
		checkerboard_depth_stat
	DESTINATION kinect
)
install(
	PROGRAMS
		"src/kinect/import_raw_data.py"
	DESTINATION kinect
)
install_pylib_link(kinect)

if(WITH_LIB${FREENECT2_LIBRARY})
	add_executable(fetch_internal_parameters "src/kinect/fetch_internal_parameters.cc")
	target_link_libraries(fetch_internal_parameters common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(viewer "src/kinect/viewer.cc")
	target_link_libraries(viewer common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(ir_distortion_viewer "src/kinect/ir_distortion_viewer.cc")
	target_link_libraries(ir_distortion_viewer common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(checkerboard_samples "src/kinect/checkerboard_samples.cc")
	target_link_libraries(checkerboard_samples common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(checkerboard_depth_samples "src/kinect/checkerboard_depth_samples.cc")
	target_link_libraries(checkerboard_depth_samples common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(checkerboard_depth_viewer "src/kinect/checkerboard_depth_viewer.cc")
	target_link_libraries(checkerboard_depth_viewer common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(checkerboard_depth_parallel "src/kinect/checkerboard_depth_parallel.cc")
	target_link_libraries(checkerboard_depth_parallel common_lib kinect_lib freenect2)

	add_executable(remapping_viewer "src/kinect/remapping_viewer.cc")
	target_link_libraries(remapping_viewer common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(reprojection_viewer "src/kinect/reprojection_viewer.cc")
	target_link_libraries(reprojection_viewer common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(ir_intrinsic_reprojection "src/kinect/ir_intrinsic_reprojection.cc")
	target_link_libraries(ir_intrinsic_reprojection common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(color_intrinsic_reprojection "src/kinect/color_intrinsic_reprojection.cc")
	target_link_libraries(color_intrinsic_reprojection common_lib kinect_lib ${FREENECT2_LIBRARY})

	add_executable(checkerboard_color_depth "src/kinect/checkerboard_color_depth.cc")
	target_link_libraries(checkerboard_color_depth common_lib kinect_lib ${FREENECT2_LIBRARY})

	install(
		TARGETS
			fetch_internal_parameters
			viewer
			ir_distortion_viewer
			checkerboard_samples
			checkerboard_depth_samples
			checkerboard_depth_viewer
			checkerboard_depth_parallel
			remapping_viewer
			reprojection_viewer
			ir_intrinsic_reprojection
			color_intrinsic_reprojection
			checkerboard_color_depth
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

add_executable(view_distortion "src/misc/view_distortion.cc")
target_link_libraries(view_distortion common_lib)

add_executable(cat_obj_img_cors "src/misc/cat_obj_img_cors.cc")
target_link_libraries(cat_obj_img_cors common_lib)

install(
	TARGETS
		psnr
		view_depth
		copy_json
		cam_rotation
		view_distortion
		cat_obj_img_cors
	DESTINATION misc
)
install(
	PROGRAMS
		"src/misc/list_skip_n_experiments.py"
		"src/misc/list_increase_baseline_experiments.py"
		"src/misc/yuv2png.sh"
	DESTINATION misc
)
install_pylib_link(misc)
