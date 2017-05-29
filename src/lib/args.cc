#include "args.h"
#include <iostream>
#include <cstdlib>
#include "misc.h"
#include "filesystem.h"

namespace tlz {

args_list::args_list(int argc, const char* argv[], const std::string& usage) :
	executable_name_(argv[0]),
	usage_(usage),
	arg_index_(0)
{
	for(int i = 1; i < argc; ++i) args_.push_back(argv[i]);
}
	
void args_list::usage_fail(const std::string& error) const {
	std::cerr << "usage: " << executable_name_ << ' ' << usage_ << std::endl;
	if(! error.empty()) std::cerr << "       " << error << std::endl;
	std::exit(EXIT_FAILURE);
}

const char* args_list::next_arg_() {
	if(arg_index_ >= args_.size())
		usage_fail("more than " + std::to_string(args_.size()) + " arguments needed");
	return args_.at(arg_index_++);
}
	
bool args_list::next_arg_is(const std::string& val) const {
	if(arg_index_ >= args_.size())
		usage_fail("more than " + std::to_string(args_.size()) + " arguments needed");
	return (val == args_.at(arg_index_));
}

bool args_list::has_next_arg() const {
	return (arg_index_ < args_.size());
}

std::string args_list::string_arg() {
	return std::string(next_arg_());
}

std::string args_list::in_filename_arg() {
	const std::string& filename = string_arg();
	if(!file_exists(filename)) usage_fail("file " + filename + " does not exist");
	return filename;
}

std::string args_list::out_filename_arg() {
	const std::string& filename = string_arg();
	return filename;
}

long args_list::int_arg() {
	const char* str = next_arg_();
	char* str_end;
	long i = std::strtol(str, &str_end, 10);
	if(str_end == str) usage_fail("`" + std::string(str) + "` is not an integer");
	return i;
}

double args_list::real_arg() {
	const char* str = next_arg_();
	char* str_end;
	double f = std::strtod(str, &str_end);
	if(str_end == str) usage_fail("`" + std::string(str) + "` is not a real");
	return f;
}

std::string args_list::enum_arg(const std::vector<std::string>& options) {
	const char* str = next_arg_();
	for(const std::string& option : options)
		if(str == option) return option;
	usage_fail("`" + std::string(str) + "` invalid value");
}

bool args_list::bool_arg(const std::string& expected) {
	const char* str = next_arg_();
	return (str == expected);
}

}
