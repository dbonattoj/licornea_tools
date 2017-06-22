#include "os.h"
#if defined(LICORNEA_OS_LINUX) || defined(LICORNEA_OS_DARWIN)
#include "filesystem.h"
#include "assert.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <limits.h>
#include <iostream>
#include <deque>
#include <system_error>

namespace tlz {

namespace {
	constexpr std::size_t path_max_length = 1024;
}

std::string filename_parent(const std::string& path_) {
	std::string path(path_);
	if(path.back() == '/') path = path.substr(0, path.length() - 1);
	auto pos = path.find_last_of('/');
	if(pos != std::string::npos) return path.substr(0, pos + 1);
	else if(path != "." && path != ".." && path != "") return ".";
	else throw std::invalid_argument("cannot get parent path of " + path);
}

std::string filename_append(const std::string& a, const std::string& b) {
	if(a.back() == '/') return a + b;
	else return a + "/" + b;
}

std::string get_current_directory() {
	char buffer[path_max_length];
	char* path = getcwd(buffer, path_max_length);
	if(path != nullptr) return std::string(path);
	else throw std::system_error(errno, std::system_category(), "getcwd() failed");
}

bool file_exists(const std::string& filename) {
	struct stat sb;
	return (stat(filename.c_str(), &sb) == 0);
}

bool is_directory(const std::string& filename) {
	struct stat sb;
	int result = stat(filename.c_str(), &sb);
	if(result == 0) return S_ISDIR(sb.st_mode);
	else throw std::system_error(errno, std::system_category(), "stat() failed");
}

bool is_file(const std::string& filename) {
	struct stat sb;
	int result = stat(filename.c_str(), &sb);
	if(result == 0) return S_ISREG(sb.st_mode);
	else throw std::system_error(errno, std::system_category(), "stat() failed");
}

std::size_t file_size(const std::string& filename) {
	struct stat sb;
	int result = stat(filename.c_str(), &sb);
	if(result == 0) return sb.st_size;
	else throw std::system_error(errno, std::system_category(), "stat() failed");
}

void make_directory(const std::string& dirname) {
	// don't fail if *directory* already exists at dirname
	
	int result = mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
	if(result == 0 || (result == -1 && errno == EEXIST && is_directory(dirname))) return;
	else throw std::system_error(errno, std::system_category(), "mkdir() failed");
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

void delete_file(const std::string& filename) {
	int result = unlink(filename.c_str());
	if(result != 0) throw std::system_error(errno, std::system_category(), "unlink() failed");
}

}

#endif
