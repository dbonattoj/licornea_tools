#include "os.h"
#if defined(LICORNEA_OS_LINUX) || defined(LICORNEA_OS_DARWIN)
#include "filesystem.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <limits.h>
#include <iostream>
#include <deque>

namespace tlz {

bool file_exists(const std::string& filename) {
	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0);
}

bool is_directory(const std::string& filename) {
	struct stat buffer;
	if(stat(filename.c_str(), &buffer) == 0) {
		return S_ISDIR(buffer.st_mode);
	} else {
		throw std::runtime_error("stat(" + filename + ") failed (does not exist?)");
	}
}

void make_directory(const std::string& dirname) {
	// don't fail if *directory* already exists at dirname
	
	int result = mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
	if(result == 0 || (result == -1 && errno == EEXIST && is_directory(dirname))) return;
	else throw std::runtime_error("mkdir(" + dirname + ") failed: " + std::to_string(result));
}



std::string filename_parent(const std::string& path_) {
	std::string path(path_);
	if(path.back() == '/') path = path.substr(0, path.length() - 1);
	auto pos = path.find_last_of('/');
	if(pos != std::string::npos) return path.substr(0, pos + 1);
	else if(path != "." && path != ".." && path != "") return ".";
	else throw std::invalid_argument("cannot get parent path of " + path);
}


void make_parent_directories(const std::string& filename) {
	std::deque<std::string> new_dirs;
	
	std::string dir = filename_parent(filename);
	while(! file_exists(dir) && dir != "/") {
		new_dirs.push_front(dir);
		dir = filename_parent(dir);
	}
	for(const std::string& dir : new_dirs) {
		make_directory(dir);
	}
}

}


#endif
