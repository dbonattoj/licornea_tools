cmake_minimum_required(VERSION 3.1)
project(pointcloud CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

include_directories(SYSTEM ../external/include)
include_directories(SYSTEM ../external/mf)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

add_compile_options(-Wall -Wextra -Wno-sign-compare)

file(GLOB_RECURSE MF_SRC "../external/mf/*.cc")
file(GLOB_RECURSE SRC "src/*.cc")

add_executable(pointcloud ${SRC} ${MF_SRC})
target_link_libraries(pointcloud ${OpenCV_LIBRARIES})
