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
	std::string string_opt_arg(const std::string& def = "")
		{ return has_next_arg() ? string_arg() : def; }
		
	std::string in_filename_arg();
	std::string in_filename_opt_arg(const std::string& def);
		{ return has_next_arg() ? in_filename_arg() : def; }

	std::string out_filename_arg();
	std::string out_filename_opt_arg(const std::string& def)
		{ return has_next_arg() ? out_filename_arg() : def; }
	
	long int_arg();
	long int_opt_arg(long def)
		{ return has_next_arg() ? int_arg() : def; }
	
	double real_arg();
	double real_opt_arg(double def)
		{ return has_next_arg() ? real_arg() : def; }

	std::string enum_arg(const std::vector<std::string>& options);
	std::string enum_opt_arg(const std::vector<std::string>& options, const std::string& def);
		{ return has_next_arg() ? enum_arg(options) : def; }

	bool bool_arg(const std::string& expected);
	bool bool_opt_arg(const std::string& expected, bool def = false);
		{ return has_next_arg() ? bool_arg(expected) : def; }
};

inline args_list get_args(int argc, const char* argv[], const std::string& usage = "") {
	return args_list(argc, argv, usage);
}

}

#endif
