#include "misc.h"
#include <cstdio>

namespace tlz {

bool file_exists(const std::string& filename) {
	if(std::FILE* file = std::fopen(filename.c_str(), "r")) {
		std::fclose(file);
		return true;
	} else {
		return false;
	}
}

}