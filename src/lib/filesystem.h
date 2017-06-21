#ifndef LICORNEA_UTILITY_FILESYSTEM_H_
#define LICORNEA_UTILITY_FILESYSTEM_H_

#include <string>
	
namespace tlz {
	
std::string filename_parent(const std::string& filename);
std::string filename_append(const std::string& a, const std::string& b);
std::string get_current_directory();

bool file_exists(const std::string& filename);

bool is_directory(const std::string& filename);
bool is_file(const std::string& filename);

void make_directory(const std::string& dirname);
void make_parent_directories(const std::string& filename);
void delete_file(const std::string& filename);


}

#endif
