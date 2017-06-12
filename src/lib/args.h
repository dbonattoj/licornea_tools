#ifndef LICORNEA_ARGS_H_
#define LICORNEA_ARGS_H_

#include "common.h"
#include <string>
#include <vector>
#include <memory>

namespace tlz {

class args_list {
private:
	const char* executable_name_;
	std::vector<const char*> args_;
	std::string usage_;
	std::ptrdiff_t arg_index_ = 0;
		
public:
	static std::unique_ptr<args_list> instance;
	args_list(int argc, const char* argv[], const std::string& usage);

	[[noreturn]] void usage_fail(const std::string& error = "") const;	
	
	bool next_arg_is(const std::string& val) const;
	bool has_next_arg() const;
	const char* next_arg();
};


args_list& args();
void get_args(int argc, const char* argv[], const std::string& usage);

bool batch_mode();

std::string string_arg();
inline std::string string_opt_arg(const std::string& def = "")
	{ return (args().has_next_arg() ? string_arg() : def); }

std::string in_filename_arg();
inline std::string in_filename_opt_arg(const std::string& def = "")
	{ return (args().has_next_arg() ? in_filename_arg() : def); }

std::string out_filename_arg();
inline std::string out_filename_opt_arg(const std::string& def = "")
	{ return (args().has_next_arg() ? out_filename_arg() : def); }

long int_arg();
inline long int_opt_arg(long def)
	{ return (args().has_next_arg() ? int_arg() : def); }

double real_arg();
inline double real_opt_arg(double def)
	{ return (args().has_next_arg() ? real_arg() : def); }

std::string enum_arg(const std::vector<std::string>& options);
inline std::string enum_opt_arg(const std::vector<std::string>& options, const std::string& def)
	{ return (args().has_next_arg() ? enum_arg(options) : def); }

bool bool_arg(const std::string& expected);
inline bool bool_opt_arg(const std::string& expected, bool def = false)
	{ return (args().has_next_arg() ? bool_arg(expected) : def); }

}

#endif
