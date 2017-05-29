#include "os.h"
#if defined(LICORNEA_OS_LINUX) || defined(LICORNEA_OS_DARWIN)
#include "filesystem.h"
#include <sys/stat.h>
#include <unistd.h>

namespace tlz {

bool file_exists(const std::string& filename) {
	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0);
}

void make_directory(const std::string& dirname) {
	mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
}

}


#endif
