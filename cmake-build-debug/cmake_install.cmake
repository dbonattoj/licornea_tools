# Install script for directory: /home/timlenertz/Desktop/licornea_tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./" TYPE DIRECTORY FILES "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib" REGEX "/[^/]*\\.so$" PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE GROUP_EXECUTE)
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/libcommon_lib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libcommon_lib.so")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/calibration" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/find_chessboard")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/find_chessboard")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/calibration" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/calibrate_distortion")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_distortion")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/calibration" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/undistort")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/undistort")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/calibration" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/calibrate_extrinsic")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/calibration/calibrate_extrinsic")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/camera" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/export_mpeg")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/export_mpeg")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/camera" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/import_mpeg")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/import_mpeg")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/camera" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/transform")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/transform")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/camera" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/visualize")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/camera/visualize")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/camera" TYPE PROGRAM FILES
    "/home/timlenertz/Desktop/licornea_tools/camera/import_xml.py"
    "/home/timlenertz/Desktop/licornea_tools/camera/make_regular_row.py"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/fetch_intrinsics")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/fetch_intrinsics")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/color_point_cloud")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/color_point_cloud")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/depth_point_cloud")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_point_cloud")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/depth_reprojection")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/depth_reprojection")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/texture_reprojection")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/texture_reprojection")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/point_cloud_reprojection")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/point_cloud_reprojection")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/vsrs_disparity")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/kinect/vsrs_disparity")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/kinect" TYPE PROGRAM FILES
    "/home/timlenertz/Desktop/licornea_tools/kinect/make_vsrs_data.py"
    "/home/timlenertz/Desktop/licornea_tools/kinect/make_vsrs_data_simulate.py"
    "/home/timlenertz/Desktop/licornea_tools/kinect/depth_cross_reprojection.py"
    "/home/timlenertz/Desktop/licornea_tools/kinect/collect_col.py"
    "/home/timlenertz/Desktop/licornea_tools/kinect/collect_fig8.py"
    "/home/timlenertz/Desktop/licornea_tools/kinect/multiprojection_window.py"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/misc" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/psnr")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/psnr")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth"
         RPATH "/usr/local/lib/")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/misc" TYPE EXECUTABLE FILES "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/view_depth")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth"
         OLD_RPATH "/home/timlenertz/Desktop/licornea_tools/external/freenect2/lib:/home/timlenertz/Desktop/licornea_tools/cmake-build-debug:"
         NEW_RPATH "/usr/local/lib/")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/misc/view_depth")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/misc" TYPE PROGRAM FILES
    "/home/timlenertz/Desktop/licornea_tools/misc/list_skip_n_experiments.py"
    "/home/timlenertz/Desktop/licornea_tools/misc/make_vsrs_config.py"
    "/home/timlenertz/Desktop/licornea_tools/misc/run_vsrs_experiments.py"
    "/home/timlenertz/Desktop/licornea_tools/misc/run_vsrs.py"
    "/home/timlenertz/Desktop/licornea_tools/misc/yuv2png.sh"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
