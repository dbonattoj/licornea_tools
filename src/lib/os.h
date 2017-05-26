#ifndef LICORNEA_UTILITY_OS_H_
#define LICORNEA_UTILITY_OS_H_

// macros for operating system
#if defined(__APPLE__)
	#define LICORNEA_OS_DARWIN
#elif defined(_WIN32)
	#define LICORNEA_OS_WINDOWS
#elif defined(__linux__)
	#define LICORNEA_OS_LINUX
#else
	#error Unknown operating system
#endif

#endif
