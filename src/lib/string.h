#ifndef LICORNEA_UTILITY_STRING_H_
#define LICORNEA_UTILITY_STRING_H_

#include <string>
#include <vector>

namespace tlz {

std::string file_name_extension(const std::string& filename);

template<typename T> std::string to_string(const T&);
template<typename T> T from_string(const std::string&);

template<typename It> std::string to_string(It begin, It end, const std::string& separator = ", ");

std::vector<std::string> explode(char separator, const std::string&);
std::string implode(char separator, const std::vector<std::string>&);

template<typename T>
std::vector<T> explode_from_string(char separator, const std::string&);

template<typename T>
std::string implode_to_string(char separator, const std::vector<T>&);

std::string to_lower(const std::string&);
std::string to_upper(const std::string&);

std::string replace_all(const std::string& subject, const std::string& find, const std::string& replace);
std::size_t replace_all_inplace(std::string& subject, const std::string& find, const std::string& replace);

int string_hash(const std::string&);


}

#include "string.tcc"

#endif
