/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef VS_RS_CONFIG_READER_H_
#define VS_RS_CONFIG_READER_H_

#include <string>
#include <map>
#include <iosfwd>
#include "../common.h"

namespace mf {

class rs_config_reader {
private:
	std::map<std::string, std::string> parameters_;
	
	void parse_(std::istream&);
	
public:
	explicit rs_config_reader(const std::string& filename);
	
	bool has(const std::string& key) const;
	const std::string& get_string(const std::string& key) const;
	int get_int(const std::string& key) const;
	mf::real get_real(const std::string& key) const;
};

}

#endif
