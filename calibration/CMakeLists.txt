cmake_minimum_required(VERSION 3.1)
project(calibration_licornea CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

include_directories(SYSTEM ../external/include)
include_directories(SYSTEM ../external/mf)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

add_compile_options(-Wall -Wextra -Wno-sign-compare)

file(GLOB_RECURSE MF_SRC "../external/mf/*.cc")
file(GLOB_RECURSE LIB_SRC "src/lib/*.cc")

add_executable(find_chessboard "src/find_chessboard.cc" ${LIB_SRC})
target_link_libraries(find_chessboard ${OpenCV_LIBRARIES})

add_executable(calibrate_distortion "src/calibrate_distortion.cc" ${LIB_SRC})
target_link_libraries(calibrate_distortion ${OpenCV_LIBRARIES})

add_executable(undistort "src/undistort.cc" ${LIB_SRC})
target_link_libraries(undistort ${OpenCV_LIBRARIES})

add_executable(calibrate_extrinsic "src/calibrate_extrinsic.cc" ${LIB_SRC})
target_link_libraries(calibrate_extrinsic ${OpenCV_LIBRARIES})
