# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/clion-2016.3.1/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion-2016.3.1/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/timlenertz/Desktop/licornea_tools

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/timlenertz/Desktop/licornea_tools/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/vsrs_disparity.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/vsrs_disparity.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/vsrs_disparity.dir/flags.make

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o: CMakeFiles/vsrs_disparity.dir/flags.make
CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o: ../kinect/vsrs_disparity.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o -c /home/timlenertz/Desktop/licornea_tools/kinect/vsrs_disparity.cc

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/timlenertz/Desktop/licornea_tools/kinect/vsrs_disparity.cc > CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.i

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/timlenertz/Desktop/licornea_tools/kinect/vsrs_disparity.cc -o CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.s

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.requires:

.PHONY : CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.requires

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.provides: CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.requires
	$(MAKE) -f CMakeFiles/vsrs_disparity.dir/build.make CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.provides.build
.PHONY : CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.provides

CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.provides.build: CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o


# Object files for target vsrs_disparity
vsrs_disparity_OBJECTS = \
"CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o"

# External object files for target vsrs_disparity
vsrs_disparity_EXTERNAL_OBJECTS =

vsrs_disparity: CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o
vsrs_disparity: CMakeFiles/vsrs_disparity.dir/build.make
vsrs_disparity: libcommon_lib.so
vsrs_disparity: libkinect_lib.a
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_videostab.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_ts.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_superres.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_stitching.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_ocl.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_gpu.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_photo.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_legacy.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_contrib.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_video.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_objdetect.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_ml.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_calib3d.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_features2d.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_highgui.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_flann.so.2.4.9
vsrs_disparity: /usr/lib/x86_64-linux-gnu/libopencv_core.so.2.4.9
vsrs_disparity: CMakeFiles/vsrs_disparity.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/timlenertz/Desktop/licornea_tools/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable vsrs_disparity"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vsrs_disparity.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/vsrs_disparity.dir/build: vsrs_disparity

.PHONY : CMakeFiles/vsrs_disparity.dir/build

CMakeFiles/vsrs_disparity.dir/requires: CMakeFiles/vsrs_disparity.dir/kinect/vsrs_disparity.cc.o.requires

.PHONY : CMakeFiles/vsrs_disparity.dir/requires

CMakeFiles/vsrs_disparity.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/vsrs_disparity.dir/cmake_clean.cmake
.PHONY : CMakeFiles/vsrs_disparity.dir/clean

CMakeFiles/vsrs_disparity.dir/depend:
	cd /home/timlenertz/Desktop/licornea_tools/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/timlenertz/Desktop/licornea_tools /home/timlenertz/Desktop/licornea_tools /home/timlenertz/Desktop/licornea_tools/cmake-build-debug /home/timlenertz/Desktop/licornea_tools/cmake-build-debug /home/timlenertz/Desktop/licornea_tools/cmake-build-debug/CMakeFiles/vsrs_disparity.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/vsrs_disparity.dir/depend
