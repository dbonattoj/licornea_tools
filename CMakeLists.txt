cmake_minimum_required(VERSION 3.1)
project(licornea_tools CXX)

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# OpenCV
find_package(OpenCV REQUIRED)
link_libraries(${OpenCV_LIBRARIES})
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})


# OpenMP
find_package(OpenMP)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Find Freenect2, if WITH_LIBFREENECT2 is set
set(WITH_LIBFREENECT2 FALSE CACHE BOOL "Use libfreenect2")
if(WITH_LIBFREENECT2)
	include_directories(SYSTEM external/freenect2/include)
	find_library(FREENECT2_LIBRARY NAMES freenect2 libfreenect2 PATHS external/freenect2/lib)
	add_compile_options(-DLICORNEA_WITH_LIBFREENECT2)
	install(
		DIRECTORY "external/freenect2/lib"
		DESTINATION "./"
		PATTERN "*.so" PATTERN "*.dylib"
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif()


# Search paths
include_directories(SYSTEM src/external/include)
include_directories(SYSTEM src/external/fmt)

add_compile_options(-Wall -Wextra -Wno-sign-compare)



# Common library
file(GLOB_RECURSE COMMON_LIB_SRC "src/lib/*.cc")
add_library(common_lib SHARED ${COMMON_LIB_SRC} "src/external/fmt/format.cc")

target_link_libraries(common_lib ${OpenCV_LIBRARIES})
install(TARGETS common_lib DESTINATION "lib/")


# Python library install
install(
	DIRECTORY "src/lib/pylib"
	DESTINATION "./lib"
	PATTERN "*.py"
)


# Marco to link pylib/ into subdirectories
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

# Macro to build+install C++ programs
function(program name section)
	add_executable(${name} "src/${section}/${name}.cc")
	target_link_libraries(${name} common_lib ${ARGN})
	install(TARGETS ${name} DESTINATION ${section})
endfunction()

# Macro to copy/install Python programs
macro(py_program name section)
	install(PROGRAMS "src/${section}/${name}.py" DESTINATION ${section})
endmacro()


# -----------------

# Calibration
file(GLOB_RECURSE CALIBRATION_LIB_SRC "src/calibration/lib/*.cc")
add_library(calibration_lib SHARED ${CALIBRATION_LIB_SRC})
target_link_libraries(calibration_lib common_lib)
install(TARGETS calibration_lib DESTINATION "lib/")
install_pylib_link(calibration)

program(image_correspondences_info calibration calibration_lib)
program(calibrate_intrinsics calibration calibration_lib)
program(view_feature_points calibration calibration_lib)
program(undistort_image calibration calibration_lib)
program(undistort_feature_points calibration calibration_lib)
program(undistort_image_correspondences calibration calibration_lib)
program(visualize_image_correspondences calibration calibration_lib)
program(read_feature_depths calibration calibration_lib)
program(export_feature_depths calibration calibration_lib)
program(feature_weights calibration calibration_lib)
program(cg_optical_flow_cors calibration calibration_lib)
program(cg_measure_optical_flow_slopes calibration calibration_lib)
program(cg_model_optical_flow_slopes calibration calibration_lib)
program(cg_visualize_optical_flow_slopes calibration calibration_lib)
program(cg_generate_artificial calibration calibration_lib)
program(cg_estimate_rotation calibration calibration_lib)
program(cg_slopes_viewer calibration calibration_lib)
program(cg_cors_viewer_f calibration calibration_lib)
program(cg_cors_viewer_v calibration calibration_lib)
program(cg_feature_straight_depths calibration calibration_lib)
program(cg_calibrate_cors calibration calibration_lib)
program(cg_unrotate_depth_map calibration calibration_lib)
program(cg_filter_features calibration calibration_lib)
program(cg_rectify calibration calibration_lib)
program(cg_optical_flow_features calibration calibration_lib)
program(cg_choose_refgrid calibration calibration_lib)
program(homographies_to_cors calibration calibration_lib)
program(copy_cors calibration calibration_lib)
program(evaluate_rectification calibration calibration_lib)

py_program(cg_warp_images calibration)



# Camera
file(GLOB_RECURSE CAMERA_LIB_SRC "src/camera/lib/*.cc")
add_library(camera_lib SHARED ${CAMERA_LIB_SRC})
target_link_libraries(camera_lib common_lib)
install(TARGETS camera_lib DESTINATION "lib/")
install_pylib_link(camera)

program(export_mpeg camera camera_lib)
program(import_mpeg camera camera_lib)
program(transform camera camera_lib)
program(visualize camera camera_lib)

py_program(import_xml camera)
py_program(import_matlab camera)


# Kinect
file(GLOB_RECURSE KINECT_LIB_SRC "src/kinect/lib/*.cc")
add_library(kinect_lib SHARED ${KINECT_LIB_SRC})
target_link_libraries(kinect_lib common_lib)
if(WITH_LIBFREENECT2)
	target_link_libraries(kinect_lib ${FREENECT2_LIBRARY})
endif()
install(TARGETS kinect_lib DESTINATION "lib/")
install_pylib_link(kinect)

program(depth_reprojection kinect kinect_lib)
program(depth_remapping kinect kinect_lib)
program(internal_ir_intrinsics kinect kinect_lib)
program(calibrate_color_ir_reprojection kinect kinect_lib)
program(checkerboard_depth_stat kinect kinect_lib)
py_program(import_raw_data kinect)


if(WITH_LIBFREENECT2)
	program(fetch_internal_parameters kinect kinect_lib ${FREENECT2_LIBRARY})
	program(close_kinect kinect kinect_lib ${FREENECT2_LIBRARY})
	program(viewer kinect kinect_lib ${FREENECT2_LIBRARY})
	program(ir_distortion_viewer kinect kinect_lib ${FREENECT2_LIBRARY})
	program(checkerboard_samples kinect kinect_lib ${FREENECT2_LIBRARY})
	program(checkerboard_depth_samples kinect kinect_lib ${FREENECT2_LIBRARY})
	program(checkerboard_depth_viewer kinect kinect_lib ${FREENECT2_LIBRARY})
	program(checkerboard_depth_parallel kinect kinect_lib ${FREENECT2_LIBRARY})
	program(remapping_viewer kinect kinect_lib ${FREENECT2_LIBRARY})
	program(reprojection_viewer kinect kinect_lib ${FREENECT2_LIBRARY})
	program(ir_intrinsic_reprojection kinect kinect_lib ${FREENECT2_LIBRARY})
	program(color_intrinsic_reprojection kinect kinect_lib ${FREENECT2_LIBRARY})
	program(checkerboard_color_depth kinect kinect_lib ${FREENECT2_LIBRARY})
	program(parallel_wall kinect kinect_lib ${FREENECT2_LIBRARY})
endif()


# Dataset
install_pylib_link(dataset)

program(view_dataset dataset)
program(duplicates dataset)
py_program(slice dataset)
py_program(flip dataset)



# VSRS
install_pylib_link(vsrs)

program(vsrs_disparity vsrs)
py_program(export_for_vsrs vsrs)
py_program(make_vsrs_config vsrs)
py_program(run_vsrs_experiments vsrs)
py_program(run_vsrs vsrs)



# Misc
install_pylib_link(misc)

program(psnr misc)
program(view_depth misc)
program(copy_json misc)
program(cam_rotation misc)
program(view_distortion misc)
program(view_syn misc)
program(cat_obj_img_cors misc)
program(apply_homography misc)
program(homography_maximal_border misc)
program(homography_crop_border misc)
program(touch misc)

py_program(list_skip_n_experiments misc)
py_program(list_increase_baseline_experiments misc)
py_program(list_skip_n_experiments misc)
