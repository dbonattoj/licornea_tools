cmake_minimum_required(VERSION 3.1)
project(licornea_tools CXX)

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# OpenCV
find_package(OpenCV REQUIRED)
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

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options(/wd4244 /wd4018 /wd4267 /wd4800 /wd4996 /wd4305)
else()
	add_compile_options(-Wall -Wextra -Wno-sign-compare)
endif()

add_definitions(-DFMT_HEADER_ONLY) # works cross-platform



# Common library
file(GLOB_RECURSE COMMON_LIB_SRC "src/lib/*.cc")
add_library(common_lib SHARED ${COMMON_LIB_SRC})
target_link_libraries(common_lib ${OpenCV_LIBRARIES})
set_target_properties(common_lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)


if(NOT WIN32)
	install(TARGETS common_lib DESTINATION "lib/")

	install(
		DIRECTORY "src/lib/pylib"
		DESTINATION "./lib"
		PATTERN "*.py"
	)
endif()


# Marco to link pylib/ into subdirectories
function(install_section_libraries section)
	# Link pylib/ into subdirectories
	if(WIN32)
		install(
			DIRECTORY "src/lib/pylib"
			DESTINATION "${section}"
			PATTERN "*.py"
		)
		install(TARGETS common_lib DESTINATION "${section}/")
		if(TARGET "${section}_lib")
			install(TARGETS "${section}_lib" DESTINATION "${section}/")
		endif()
	else()
		install(
			CODE "execute_process( \
				WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX} \
				COMMAND ${CMAKE_COMMAND} -E create_symlink \
				../lib/pylib \
				${section}/pylib \
			)"
		)
		if(TARGET "${section}_lib")
			install(TARGETS "${section}_lib" DESTINATION "lib/")
		endif()
	endif()
endfunction(install_section_libraries)

# Macro to build+install C++ programs
function(program name section)
	add_executable(${name} "src/${section}/${name}.cc")
	target_link_libraries(${name} common_lib ${OpenCV_LIBRARIES} ${ARGN})
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
set_target_properties(calibration_lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
install_section_libraries(calibration)

program(cors_info calibration calibration_lib)
program(calibrate_intrinsics calibration calibration_lib)
program(cors_view_fpoints calibration calibration_lib)
program(undistort_image calibration calibration_lib)
program(undistort_fpoints calibration calibration_lib)
program(undistort_cors calibration calibration_lib)
program(read_feature_depths calibration calibration_lib)
program(export_feature_depths calibration calibration_lib)
program(cg_optical_flow_cors calibration calibration_lib)
program(cg_measure_optical_flow_slopes calibration calibration_lib)
program(cg_model_optical_flow_slopes calibration calibration_lib)
program(cg_visualize_fslopes calibration calibration_lib)
program(cg_generate_artificial calibration calibration_lib)
program(cg_rotation_from_fslopes calibration calibration_lib)
#program(cg_rotation_from_depths calibration calibration_lib)
program(cg_slopes_viewer calibration calibration_lib)
program(cg_cors_viewer_f calibration calibration_lib)
program(cg_cors_viewer_v calibration calibration_lib)
program(cg_straight_depths_from_depths calibration calibration_lib)
#program(cg_straight_depths_from_disparity calibration calibration_lib)
program(cg_filter_features calibration calibration_lib)
program(cg_optical_flow_features calibration calibration_lib)
program(cg_choose_refgrid calibration calibration_lib)
program(cg_rcpos_from_cors calibration calibration_lib)
program(cg_stitch_cameras calibration calibration_lib)
program(cg_redistribute_cors calibration calibration_lib)
program(cg_compare_straight_depths calibration calibration_lib)
program(copy_cors calibration calibration_lib)
program(evaluate_calibration calibration calibration_lib)
program(merge_cors calibration calibration_lib)
program(remove_cors calibration calibration_lib)
program(cameras_from_checkerboards calibration calibration_lib)



# Camera
file(GLOB_RECURSE CAMERA_LIB_SRC "src/camera/lib/*.cc")
add_library(camera_lib SHARED ${CAMERA_LIB_SRC})
target_link_libraries(camera_lib common_lib)
set_target_properties(camera_lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
install_section_libraries(camera)

program(export_mpeg camera camera_lib)
program(import_mpeg camera camera_lib)
program(transform camera camera_lib)
program(visualize camera camera_lib)
program(merge_cameras camera camera_lib)
program(restrict_cameras camera camera_lib)

py_program(import_xml camera)
py_program(import_matlab camera)


# Kinect
file(GLOB_RECURSE KINECT_LIB_SRC "src/kinect/lib/*.cc")
add_library(kinect_lib SHARED ${KINECT_LIB_SRC})
target_link_libraries(kinect_lib common_lib)
if(WITH_LIBFREENECT2)
	target_link_libraries(kinect_lib ${FREENECT2_LIBRARY})
endif()
set_target_properties(kinect_lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
install_section_libraries(kinect)

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
install_section_libraries(dataset)

program(view_dataset dataset)
program(duplicates dataset)
program(missing dataset)
py_program(slice dataset)
py_program(flip dataset)



# VSRS
install_section_libraries(vsrs)

program(vsrs_disparity vsrs)
py_program(export_for_vsrs vsrs)
py_program(make_vsrs_config vsrs)
py_program(run_vsrs_experiments vsrs)
py_program(run_vsrs vsrs)
py_program(psnr_experiments vsrs)
py_program(list_skip_n_experiments vsrs)
py_program(list_increase_baseline_experiments vsrs)
py_program(list_parametric_experiments vsrs)



# Misc
install_section_libraries(misc)

program(psnr misc)
program(view_depth misc)
program(copy_json misc)
program(cam_rotation misc)
program(view_distortion misc)
program(view_syn misc)
program(cat_obj_img_cors misc)
program(apply_homography misc)
program(homography_maximal_border misc)
program(touch misc)
program(yuv_import misc)
program(yuv_export misc)

py_program(extract_parametric misc)
