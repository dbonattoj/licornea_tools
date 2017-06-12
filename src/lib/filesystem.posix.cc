#include "os.h"
#if defined(LICORNEA_OS_LINUX) || defined(LICORNEA_OS_DARWIN)
#include "filesystem.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include <limits.h>
#include <iostream>

namespace tlz {

bool file_exists(const std::string& filename) {
	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0);
}

void make_directory(const std::string& dirname) {
	int result = mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
	if(result != 0) throw std::runtime_error("mkdir(" + dirname + ") failed");
}


std::string filename_parent(const std::string& path_) {
	std::string path(path_);
	if(path.back() == '/') path = path.substr(0, path.length() - 1);
	auto pos = path.find_last_of('/');
	if(pos != std::string::npos) return path.substr(0, pos + 1);
	else throw std::runtime_error("cannot get parent of " + path);
}


void make_parent_directories(const std::string& filename) {
	std::string dir = filename_parent(filename);
	while(! file_exists(dir) && dir != "/") {
		make_directory(dir);
		dir = filename_parent(dir);
	}
}

}


#endif
