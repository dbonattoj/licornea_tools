#include "os.h"
#if defined(LICORNEA_OS_DARWIN) || defined(LICORNEA_OS_LINUX)

#include "filesystem.h"
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <ostream>

namespace tlz {
	
std::string file_path::filename() const {
	auto pos = native_path_.find_last_of('/');
	if(pos != std::string::npos) return native_path_.substr(pos + 1);
	else return native_path_;
}

std::string file_path::stem() const {
	std::string name = filename();
	auto pos = name.find_last_of('.');
	if(pos != std::string::npos) return name.substr(0, pos);
	else return name;
}

std::string file_path::extension() const {
	std::string name = filename();
	auto pos = name.find_last_of('.');
	if(pos != std::string::npos) return name.substr(pos + 1);
	else return "";
}

file_path file_path::parent() const {
	std::string path = native_path_;
	if(path.back() == '/') path = path.substr(0, path.length() - 1);
	auto pos = path.find_last_of('/');
	if(pos != std::string::npos) return path.substr(0, pos + 1);
	else return ".";
}

file_path file_path::concat(const file_path& rel_path) const {
	std::string this_path = native_path_;
	if(this_path.back() != '/') this_path += '/';
	return this_path + rel_path.native_path_;
}

bool file_path::is_absolute() const {
	return (native_path_.front() == '/');
}

file_path file_path::canonical() const {
	char buffer[PATH_MAX];
	char* path = realpath(native_path_.c_str(), buffer);
	if(path == nullptr) throw std::runtime_error("realpath failed");
	else return path;
}

file_path current_working_directory() {
	char buffer[PATH_MAX];
	char* path = getcwd(buffer, sizeof buffer);
	if(path == nullptr) throw std::runtime_error("getcwd failed");
	return file_path(path);
}

bool exists(const file_path& path) {
	int result = access(path.c_str(), F_OK);
	return (result == 0);
}

bool is_file(const file_path& path) {
	struct stat path_stat;
	stat(path.c_str(), &path_stat);
	return S_ISREG(path_stat.st_mode);
}

bool is_directory(const file_path& path) {
	struct stat path_stat;
	stat(path.c_str(), &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void make_parent_directories(const file_path& path_) {
	file_path dir = path_.canonical().parent();
	while(! exists(dir) && dir.native() != "/") {
		int result = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
		if(result != 0) throw std::runtime_error("mkdir(" + dir.native() + ") failed");
	}
}

void remove(const file_path& path) {
	if(is_directory(path)) {
		int result = rmdir(path.c_str());
		if(result != 0) throw std::runtime_error("rmdir(" + path.native() + ") failed");
	} else {
		int result = unlink(path.c_str());
		if(result != 0) throw std::runtime_error("unlink(" + path.native() + ") failed");
	}
}

std::ostream& operator<<(std::ostream& str, const file_path& path) {
	str << path.native();
	return str;
}

}

#endif
