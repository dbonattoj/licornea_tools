# Installation and usage

The tools consists of several small command-line programs written in C++ and Python. Installation compiles the C++ programs, and copies the binary along with the Python scripts into the installation directory.

## Requirements
All of C++ programs require [**OpenCV**](http://opencv.org) to be installed on the system.

The programs that operate with the **Kinect v2** require [**libfreenect2**](https://github.com/OpenKinect/libfreenect2). The rest of the programs can still be built if it is not available.

To enable parallelization in the C++ programs, the compiler must support [**OpenMP**](https://en.wikipedia.org/wiki/OpenMP). If it is not available, the programs cannot run parallelized.

For the parallelization of the Python batch progresses, the Python extension [**joblib**](https://pythonhosted.org/joblib/) must be installed. (Using `pip`, for example.)

Building is done with [**CMake**](https://cmake.org). The C++ programs require a C++14 compatible compiler. It has been tested with a recent version of Clang++/LLVM on Linux. Other than the C++ standard library, and some external libraries that are included with the source code, there are no additional dependencies.

## Installation

### On Linux or macOS
1. Install CMake and OpenCV on the system.
2. (Optional) Install libfreenect2. It needs to be compiled from source. When building libfreenect2 with its CMake script, `CMAKE_INSTALL_PREFIX` needs to be set to `path/to/licornea_tools/external/freenect2`. `make install` then copies its library and header files into that directory, where the `licornea_tools` CMake script will find them.
3. Go to `licornea_tools/` directory.
4. `mkdir build; cd build`
5. `cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../bin ..`. If libfreenect2 was installed, also pass `-DWITH_LIBFREENECT2=ON`. Possibly the OpenCV installation directory also needs to be adjusted using `-DOpenCV_DIR=...` (or similar depending on system).
6. Build using `make`. There may be some warnings, and some errors that need to be fixed in the code if compiling with a diffent platform/compiler. There may be `rpath` issues on macOS.
7. Install using `make install`. The tools will be installed in `licornea_tools/bin`, Python scripts (copies) along with executables, in their subdirectories.
8. Make sure that the executables are properly linked. Each subdirectory must contain a symlink to `bin/pylib/`, and they must link to the shared libraries in `bin/lib/`, and OpenCV, etc.

### On Windows
1. Install CMake, OpenCV and Visual Studio C++ on the system. Use a command prompt that gives access to the native Visual Studio tools, for example "MSBuild Command Prompt for VS2015".
2. (Optional) Install libfreenect2. It needs to be compiled from source. When building libfreenect2 with its CMake script, `CMAKE_INSTALL_PREFIX` needs to be set to `path/to/licornea_tools/external/freenect2`.
3. Go to `licornea_tools/` directory.
4. `mkdir build; cd build`
5. `cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=..\bin -DOpenCV_DIR=... ..`. If libfreenect2 was installed, also pass `-DWITH_LIBFREENECT2=ON`. `OpenCV_DIR` needs to be set to the root directory of the OpenCV installation. Possibly use another version of Visual Studio as generator.
6. Add the OpenCV `bin\` directory to the system-wide `PATH` environment variable, as prompted by the CMake output.
7. Build using `cmake --build . --config Release --target ALL_BUILD`.
8. Install using `cmake --build . --config Release --target INSTALL`.
9. Make sure that the executables are properly linked. Each subdirectory must contain a copy of `pylib/`, and the DLLs for `common_lib` and the for category.


## Usage
The programs should be used only from the installed `bin/` directory. The Python scripts cannot be called from their source files in `src/`. They rely on their libraries to be available in the right location, and some call other tools that must be in the same directories.

Each tool can be run from the command line, like for example

    camera/export_mpeg cams.json vsrs_cams.txt
    
On Windows it would be

    camera\export_mpeg.exe cams.json vsrs_cams.txt

When called with no arguments, it displays a usage notice like

    usage: export_mpeg cameras.json out_cameras_mpeg.txt [no_convert]
    
The arguments with a file extension (like `.json`) are placeholders for a relative file path to a file of that type. Output files (that will be created/written to by the program) are usually prefixed `out_`. Arguments in [brackets] are optional. Boolean argument are set by giving their name as argument, for example `no_convert`. Any other string (like `-`) disables it.

C++ programs that take an output filename automatically create the subdirectories leading to that file path automatically. If the file already exists, the program displays a asks before replacing the file. 


## Environment variables 
Some environment variables influence the behavior of the programs:

- `LICORNEA_BATCH_MODE`: C++ programs do not ask permission before replacing existing output files. Always set (to `1`) when they are called from a Python program.
- `LICORNEA_VERBOSE`: For Python programs only, whether to print additional (debug) output.
- `LICORNEA_PARALLEL`: For Python programs only, parallelized execution of batch processes is enables when set to `1`. 
- `LICORNEA_NUM_THREADS`: For Python programs only, number of threads for parallelized batch execution.
- `OMP_NUM_THREADS`, etc: Configures OpenMP parallelization.
