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

#ifndef MF_YUV_IMPORTER_H_
#define MF_YUV_IMPORTER_H_

#include <fstream>
#include <memory>
#include "../color.h"
#include "../nd.h"

namespace mf {

class yuv_importer {
private:
	using char_type = std::ifstream::char_type;

	std::ifstream file_;
	ndsize<2> frame_shape_;
	std::size_t file_size_;
	std::size_t current_time_ = 0;
	
	std::unique_ptr<char_type[]> frame_buffer_;
	std::streamsize frame_size_;
	ndarray_view<2, char_type> y_view_;
	ndarray_view<2, char_type> cb_view_;
	ndarray_view<2, char_type> cr_view_;
	
	std::size_t chroma_scale_y_;
	std::size_t chroma_scale_x_;
	
public:
	static int Animation_frame;

	yuv_importer(const std::string& filename, const ndsize<2>& frame_shape, int sampling);
		
	void read_frame(const ndarray_view<2, ycbcr_color>&);
	bool reached_end() const;

	time_unit current_time() const;
	time_unit total_duration() const;
	
	void seek(time_unit);
};
	
}

#endif
