#ifndef LICORNEA_UTILITY_FILESYSTEM_H_
#define LICORNEA_UTILITY_FILESYSTEM_H_

#include <string>
	
namespace tlz {
	
bool file_exists(const std::string& filename);

void make_directory(const std::string& dirname);

}

#endif
