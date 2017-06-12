#include "args.h"
#include <iostream>
#include <cstdlib>
#include "misc.h"
#include "filesystem.h"

namespace tlz {

std::unique_ptr<args_list> args_list::instance = std::unique_ptr<args_list>();

args_list& args() {
	if(args_list::instance) return *args_list::instance;
	else throw std::logic_error("args_list not initialized (get_args was not called)");
}


void get_args(int argc, const char* argv[], const std::string& usage) {
	args_list::instance = std::make_unique<args_list>(argc, argv, usage);
}


bool batch_mode() {
	const char* batch_mode_env = std::getenv("LICORNEA_BATCH_MODE");
	if(batch_mode_env == nullptr) return false;
	else return (std::string(batch_mode_env) == "1");
}

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

const char* args_list::next_arg() {
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

std::string string_arg() {
	return std::string(args().next_arg());
}

std::string in_filename_arg() {
	const std::string& filename = string_arg();
	if(!file_exists(filename)) args().usage_fail("file " + filename + " does not exist");
	return filename;
}

std::string out_filename_arg() {
	const std::string& filename = string_arg();
	make_parent_directories(filename);
	if(file_exists(filename)) {
		if(batch_mode()) {
			std::cout << "overwriting existing output file " << filename << std::endl;
		} else {
			std::cout << "output file " << filename << " exists. Proceed? [y/n] ";
			char answer;
			std::cin >> answer;
			if(answer != 'y' && answer != 'Y') throw std::runtime_error("not overwriting output file, exiting");
		}
	}
	return filename;
}

long int_arg() {
	const char* str = args().next_arg();
	char* str_end;
	long i = std::strtol(str, &str_end, 10);
	if(str_end == str) args().usage_fail("`" + std::string(str) + "` is not an integer");
	return i;
}

double real_arg() {
	const char* str = args().next_arg();
	char* str_end;
	double f = std::strtod(str, &str_end);
	if(str_end == str) args().usage_fail("`" + std::string(str) + "` is not a real");
	return f;
}

std::string enum_arg(const std::vector<std::string>& options) {
	const char* str = args().next_arg();
	for(const std::string& option : options)
		if(str == option) return option;
	args().usage_fail("`" + std::string(str) + "` invalid value");
}

bool bool_arg(const std::string& expected) {
	const char* str = args().next_arg();
	return (str == expected);
}

}
