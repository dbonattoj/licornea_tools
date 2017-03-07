cmake_minimum_required(VERSION 3.1)
project(kinect_licornea CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

include_directories(SYSTEM ../external/include)
include_directories(SYSTEM ../external/freenect2/include)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
link_directories(../external/freenect2/lib)

add_compile_options(-Wall -Wextra -Wno-sign-compare)

file(GLOB_RECURSE LIB_SRC "src/lib/*.cc")

add_executable(fetch_intrinsics "src/fetch_intrinsics.cc" ${LIB_SRC})
target_link_libraries(fetch_intrinsics ${OpenCV_LIBRARIES} freenect2)

add_executable(depth_reprojection "src/depth_reprojection.cc" ${LIB_SRC})
target_link_libraries(depth_reprojection ${OpenCV_LIBRARIES} freenect2)

add_executable(texture_reprojection "src/texture_reprojection.cc" ${LIB_SRC})
target_link_libraries(texture_reprojection ${OpenCV_LIBRARIES} freenect2)

add_executable(vsrs_disparity "src/vsrs_disparity.cc" ${LIB_SRC})
target_link_libraries(vsrs_disparity ${OpenCV_LIBRARIES})
