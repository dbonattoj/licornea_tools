#include "os.h"
#if defined(LICORNEA_OS_WINDOWS)
#include "filesystem.h"

namespace tlz {

bool file_exists(const std::string& filename) {
	return true;
}

void make_directory(const std::string& dirname) {
	throw std::runtime_error("not implemented");
}

std::string real_filename(const std::string& filename) {
	
}


std::string filename_parent(const std::string& path) {
	
}


void make_parent_directories(const std::string& filename) {
	
}

}


#endif
