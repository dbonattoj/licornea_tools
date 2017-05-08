#include "ply_exporter.h"
#include "common.h"
#include <iomanip>
#include <stdexcept>
#include <cassert>

namespace tlz {

namespace {
	constexpr std::size_t vertex_count_string_length_ = 15;
}

ply_exporter::ply_exporter(const std::string& filename, bool full, bool ascii, line_delimitor ld) :
	file_(filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary),
	line_delimitor_(ld),
	full_(full),
	ascii_(ascii)
{
	write_line_("ply");
	if(ascii_) write_line_("format ascii 1.0");
	else if(host_is_little_endian) write_line_("format binary_little_endian 1.0");
	else write_line_("format binary_big_endian 1.0");
	
	write_line_("comment PLY file generated using tlz::ply_exporter");
	
	file_ << "element vertex ";
	file_ << std::flush;
	vertex_count_string_position_ = file_.tellp();
	file_ << std::setfill(' ') << std::left << std::setw(vertex_count_string_length_) << 0;
	write_line_("");
	
	write_line_("property float x");
	write_line_("property float y");
	write_line_("property float z");
	if(full_) {
		write_line_("property uchar red");
		write_line_("property uchar green");
		write_line_("property uchar blue");
	}
	
	write_line_("end_header");
}


ply_exporter::~ply_exporter() {
	close();
}


void ply_exporter::close() {
	// write vertex count to file
	file_.seekp(vertex_count_string_position_);
	file_ << std::setfill(' ') << std::left << std::setw(vertex_count_string_length_) << count_;

	// close
	file_.close();
}


void ply_exporter::write_line_(const std::string& ln) {
	write_line(file_, ln, line_delimitor_);
}


void ply_exporter::write_binary_(const point_xyz& p) {
	float pos[3] = { p.x, p.y, p.z };
	file_.write(reinterpret_cast<const char*>(&pos), 3 * sizeof(float));
	count_++;
}


void ply_exporter::write_ascii_(const point_xyz& p) {
	file_ << p.x << ' ' << p.y << ' ' << p.z;
	end_line(file_, line_delimitor_);
	count_++;
}


void ply_exporter::write_full_binary_(const point_full& p) {
	float pos[3] = { p.x, p.y, p.z };
	rgb_color col = p.color;
	
	file_.write(reinterpret_cast<const char*>(&pos), 3 * sizeof(float));
	file_.write(reinterpret_cast<const char*>(&col), 3);
	count_++;
}


void ply_exporter::write_full_ascii_(const point_full& p) {
	file_ << p.x << ' ' << p.y << ' ' << p.z
		<< ' ' << (unsigned)p.color.r << ' ' << (unsigned)p.color.g << ' ' << (unsigned)p.color.b;
	end_line(file_, line_delimitor_);
	count_++;
}

}
