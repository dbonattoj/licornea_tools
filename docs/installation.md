# Installation and usage

The tools consists of several small command-line programs written in C++ and Python. Installation compiles the C++ programs, and copies the binary along with the Python scripts into the installation directory.

# Installation

The C++ programs require [**OpenCV**](http://opencv.org) to be installed on the system. The programs that operate with the **Kinect v2** require [**libfreenect2**](https://github.com/OpenKinect/libfreenect2). The rest of the programs can still be built if it is not available. Building is done with [**CMake**](https://cmake.org). The C++ programs require a C++14 compatible compiler. It has been tested with a recent version of Clang++/LLVM on Linux. Other than the C++ standard library, and some external libraries that are included with the source code, there are no additional dependencies.

This is the installation on Linux / macOS. On Windows, Visual Studio project files, or `NMake` makefiles can be generated with CMake instead.

1. Install CMake and OpenCV on the system.
2. (Optional) Install libfreenect2. It needs to be compiled from source. When building libfreenect2 with its CMake script, `CMAKE_INSTALL_PREFIX` needs to be set to `path/to/licornea_tools/external/freenect2`. `make install` then copies its library and header files into that directory, where the `licornea_tools` CMake script will find them.
3. Go to `licornea_tools/` top left directory.
4. `mkdir build; cd build`
5. `cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../bin ..`. If libfreenect2 was installed, also pass `-DWITH_LIBFREENECT2`. Possibly the OpenCV installation directory also needs to be adjusted using `-DOpenCV_DIR=...` (or similar depending on system).
6. For the Python scripts, there are some configuration options in `src/lib/pylib/config.py`, relating to parallelized execution of batch jobs.
7. Build using `make`. There may be some warnings, and some errors that need to be fixed in the code if compiling with a diffent platform/compiler. There may be `rpath` issues on macOS.
8. Install using `make install`. The tools will be installed in `licornea_tools/bin`, Python scripts (copies) along with executables, in their subdirectories.


# Usage

The programs should be used only from the installed `bin/` directory. The Python scripts cannot be called from their source files in `src/`. They rely on their libraries to be available in the right location, and some call other tools that must be in the same directories.

Each tool can be run from the command line, like for example

    $ camera/export_mpeg ~/data/1234/cams.json ~/data/1234/vsrs_cams.txt

When called with no arguments, it displays a usage notice like

    usage: export_mpeg cameras.json out_cameras_mpeg.txt [no_convert]
    
The arguments with a file extension (like `.json`) are placeholders for a relative file path to a file of that type. Output files (that will be created/written to by the program) are usually prefixed `out_`. Arguments in [brackets] are optional.
