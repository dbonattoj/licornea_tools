#include "os.h"
#if defined(LICORNEA_OS_WINDOWS)
#include "filesystem.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <deque>

namespace {

template<typename Char>
struct temp_string_holder {
	std::basic_string<Char> str;
	
	explicit temp_string_holder(const std::string& in_str) :
		str(in_str.begin(), in_str.end()) { }
	operator const Char* () const
		{ return str.c_str(); }
};
template<>
struct temp_string_holder<char> {
	const std::string& str;
	
	explicit temp_string_holder(const std::string& in_str) :
		str(in_str) { }
	operator const char* () const
		{ return str.c_str(); }
};

temp_string_holder<char> to_LPCSTR(const std::string& str) {
	return temp_string_holder<char>(str);
}
temp_string_holder<wchar_t> to_LPCWSTR(const std::string& str) {
	return temp_string_holder<wchar_t>(str);
}
temp_string_holder<TCHAR> to_LPCTSTR(const std::string& str) {
	return temp_string_holder<TCHAR>(str);
}

}

namespace tlz {

namespace {
	constexpr std::size_t path_max_length = 1024;
}

std::string filename_parent(const std::string& path_) {
	std::string path(path_);
	if(path.back() == '/' || path.back() == '\\') path = path.substr(0, path.length() - 1);
	auto pos = path.find_last_of("/\\");
	if(pos != std::string::npos) return path.substr(0, pos + 1);
	else if(path != "." && path != ".." && path != "" && !(path.length() <= 3 && path[1] == ':')) return ".";
	else throw std::invalid_argument("cannot get parent path of " + path);
}

std::string filename_append(const std::string& a, const std::string& b) {
	if(a.back() == '/' || a.back() == '\\') return a + b;
	else return a + "\\" + b;
}

std::string get_current_directory() {
	TCHAR buffer[path_max_length];
	DWORD length = GetCurrentDirectory(path_max_length, buffer);
	if(length == 0 || length > path_max_length)
		throw std::system_error(GetLastError(), std::system_category(), "GetCurrentDirectory() failed");
	
	return std::string(buffer, buffer + length);	
}

bool file_exists(const std::string& filename) {
	DWORD attr = GetFileAttributes(to_LPCTSTR(filename));
	return (attr != INVALID_FILE_ATTRIBUTES);
}

bool is_directory(const std::string& filename) {
	DWORD attr = GetFileAttributes(to_LPCTSTR(filename));
	if(attr == INVALID_FILE_ATTRIBUTES)
		throw std::system_error(GetLastError(), std::system_category(), "GetFileAttributes() failed");
	return (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool is_file(const std::string& filename) {
	DWORD attr = GetFileAttributes(to_LPCTSTR(filename));
	if(attr == INVALID_FILE_ATTRIBUTES)
		throw std::system_error(GetLastError(), std::system_category(), "GetFileAttributes() failed");
	return (attr == FILE_ATTRIBUTE_NORMAL);
}

std::size_t file_size(const std::string& filename) {
	std::ifstream stream(filename, std::ios_base::ate | std::ios_base::binary);
	return stream.tellg();
}

void make_directory(const std::string& dirname) {
	BOOL ret = CreateDirectory(to_LPCTSTR(dirname), NULL);
	if(ret == 0) throw std::system_error(GetLastError(), std::system_category(), "CreateDirectory() failed");
}

void make_parent_directories(const std::string& filename) {
	std::deque<std::string> new_dirs;
	
	std::string dir = filename_parent(filename);
	while(! file_exists(dir) && dir != "/" && dir != "\\") {
		new_dirs.push_front(dir);
		dir = filename_parent(dir);
	}
	for(const std::string& dir : new_dirs) {
		make_directory(dir);
	}
}

void delete_file(const std::string& filename) {
	BOOL ret = DeleteFile(to_LPCTSTR(filename));
	if(ret == 0) throw std::system_error(GetLastError(), std::system_category(), "DeleteFile() failed");
}

}


#endif
