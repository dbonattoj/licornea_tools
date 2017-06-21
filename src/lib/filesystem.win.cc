#include "os.h"
#if defined(LICORNEA_OS_WINDOWS)
#include "filesystem.h"
#include <windows.h>
#include <string>

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

LPCSTR to_LPCSTR(const std::string& str) {
	return temp_string_holder<char>(str);
}
LPCWSTR to_LPCWSTR(const std::string& str) {
	return temp_string_holder<wchar_t>(str);
}
LPCTSTR to_LPCTSTR(const std::string& str) {
	return temp_string_holder<TCHAR>(str);
}

}

namespace tlz {



}


#endif
