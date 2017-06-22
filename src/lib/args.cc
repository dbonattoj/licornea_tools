#include "args.h"
#include <iostream>
#include <cstdlib>
#include "misc.h"
#include "filesystem.h"
#include "string.h"

namespace tlz {
	
namespace {
	
std::string out_filename_(const std::string& filename) {
	if(filename == "") return "";
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

std::string out_dirname_(const std::string& dirname) {
	if(dirname == "") return "";
	if(! file_exists(dirname)) {
		make_parent_directories(dirname);
		make_directory(dirname);
	} else {
		if(! is_directory(dirname)) throw std::runtime_error("file exists at " + dirname);
	}
	return dirname;
}

}

std::unique_ptr<args_list> args_list::instance = std::unique_ptr<args_list>();

args_list& args() {
	if(args_list::instance) return *args_list::instance;
	else throw std::logic_error("args_list not initialized (get_args was not called)");
}


void get_args(int argc, const char* argv[], const std::string& usage) {
	if(argc >= 2 && std::string(argv[1]) == "--help") {
		std::cout << "usage: " << argv[0] << " " << usage << std::endl;
		std::exit(EXIT_SUCCESS);
	}
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
	if(filename == "-") throw std::runtime_error("expected filename argument, not -");
	return out_filename_(filename);
}

std::string out_filename_opt_arg(const std::string& def) {
	if(! args().has_next_arg()) {
		return out_filename_(def);
	} else if(args().next_arg_is("-")) {
		args().next_arg();
		return out_filename_(def);;
	} else {
		return out_filename_arg();
	}
}

std::string out_dirname_arg() {
	const std::string& dirname = string_arg();
	if(dirname == "-") throw std::runtime_error("expected dirname argument, not -");
	return out_dirname_(dirname);
}

std::string out_dirname_opt_arg(const std::string& def) {
	if(! args().has_next_arg()) {
		return out_dirname_(def);
	} else if(args().next_arg_is("-")) {
		args().next_arg();
		return out_dirname_(def);
	} else {
		return out_dirname_arg();
	}
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

view_index view_index_arg() {
	const std::string& str = string_arg();
	view_index idx;
	auto j_idx = explode_from_string<int>(',', str);
	idx.x = j_idx[0];
	if(j_idx.size() == 2)  idx.y = j_idx[1];
	else idx.y = -1;
	return idx;
}

}
