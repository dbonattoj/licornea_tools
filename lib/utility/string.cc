#include "string.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <cctype>

namespace tlz {

std::string file_name_extension(const std::string& filename) {
	std::ptrdiff_t pos = filename.find_last_of('.');
	if(pos == std::string::npos) return "";
	else return filename.substr(pos + 1);
}

std::vector<std::string> explode(char separator, const std::string& str) {
	std::vector<std::string> vec;
	std::string::size_type last_pos = 0;
	std::string::size_type pos = str.find(separator);
	while(pos != std::string::npos) {
		std::string::size_type n = pos - last_pos;
		std::string part = str.substr(last_pos, n);
		vec.push_back(part);
		last_pos = pos + 1;
		pos = str.find(separator, last_pos);
	}
	vec.push_back(str.substr(last_pos));
	return vec;
}


std::string implode(char separator, const std::vector<std::string>& vec) {
	std::ostringstream ostr;
	auto last = vec.end() - 1;
	for(auto it = vec.begin(); it != last; ++it) {
		ostr << *it << separator;
	}
	ostr << vec.back();
	return ostr.str();
}


std::string to_lower(const std::string& s_orig) {
	std::string s(s_orig);
	for(char& c: s) c = std::tolower(c);
	return s;
}


std::string to_upper(const std::string& s_orig) {
	std::string s(s_orig);
	for(char& c: s) c = std::toupper(c);
	return s;
}


std::string replace_all(const std::string& subject_orig, const std::string& find, const std::string& replace) {
	std::string subject = subject_orig;
	replace_all_inplace(subject, find, replace);
	return subject;
}


std::size_t replace_all_inplace(std::string& subject, const std::string& find, const std::string& replace) {
	std::size_t pos = 0;
	std::size_t count = 0;
	while( (pos = subject.find(find, pos)) != std::string::npos ) {
		subject.replace(pos, find.length(), replace);
		pos += replace.length();
		++count;
	}
	return count;
}


}
