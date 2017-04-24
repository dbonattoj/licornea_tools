#ifndef LICORNEA_UTILITY_FILESYSTEM_H_
#define LICORNEA_UTILITY_FILESYSTEM_H_

#include <string>
#include <iosfwd>
	
namespace tlz {
	
class file_path {
private:
	std::string native_path_;
	
public:
	file_path(const std::string& native) : native_path_(native) { }
	file_path(const char* native) : native_path_(native) { }

	std::string filename() const;
	std::string stem() const;
	std::string extension() const;

	const std::string& native() const { return native_path_; }
	const char* c_str() const { return native_path_.c_str(); }
	
	operator const std::string& () const { return native_path_; }
	
	file_path parent() const;
	
	file_path concat(const file_path&) const;
	file_path operator/(const file_path& sub) const { return concat(sub); }
	
	bool is_absolute() const;
	file_path canonical() const;
};

file_path current_working_directory();

bool exists(const file_path&);
bool is_file(const file_path&);
bool is_directory(const file_path&);

void make_parent_directories(const file_path&);
void remove(const file_path&);

std::ostream& operator<<(std::ostream&, const file_path&);

}

#endif
