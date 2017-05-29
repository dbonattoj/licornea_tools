#ifndef LICORNEA_ARGS_H_
#define LICORNEA_ARGS_H_

#include "common.h"
#include <string>
#include <vector>

namespace tlz {

class args_list {
private:
	const char* executable_name_;
	std::vector<const char*> args_;
	std::string usage_;
	std::ptrdiff_t arg_index_ = 0;
	
	const char* next_arg_();
	
public:
	args_list(int argc, const char* argv[], const std::string& usage);
	
	[[noreturn]] void usage_fail(const std::string& error) const;	
	
	bool next_arg_is(const std::string& val) const;
	bool has_next_arg() const;

	std::string string_arg();
	std::string filename_arg(bool must_exist);
	std::string in_filename_arg() { return filename_arg(true); }
	std::string out_filename_arg() { return filename_arg(false); }
	long int_arg();
	double real_arg();
	std::string enum_arg(const std::vector<std::string>& options);
	bool bool_arg(const std::string& expected);
};

inline args_list get_args(int argc, const char* argv[], const std::string& usage = "") {
	return args_list(argc, argv, usage);
}

}

#endif
