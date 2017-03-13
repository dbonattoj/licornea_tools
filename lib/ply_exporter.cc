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
	
	write_line_("comment PLY file generated using mf::ply_exporter");
	
	file_ << "element vertex ";
	file_ << std::flush;
	vertex_count_string_position_ = file_.tellp();
	file_ << std::setfill(' ') << std::left << std::setw(vertex_count_string_length_) << 0;
	write_line_("");
	
	std::string scalar_type_name;
	if(sizeof(Eigen_scalar) == 4) scalar_type_name = "float";
	else if(sizeof(Eigen_scalar) == 8) scalar_type_name = "double";
	else throw std::logic_error("unsupported Eigen_scalar type");
	
	write_line_("property " + scalar_type_name + " x");
	write_line_("property " + scalar_type_name + " y");
	write_line_("property " + scalar_type_name + " z");
	if(full_) {
		write_line_("property " + scalar_type_name + " nx");
		write_line_("property " + scalar_type_name + " ny");
		write_line_("property " + scalar_type_name + " nz");
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
	file_.write(reinterpret_cast<const char*>( p.position().data() ), 3 * sizeof(Eigen_scalar));
	count_++;
}


void ply_exporter::write_ascii_(const point_xyz& p) {
	Eigen_vec3 position = p.position();	
	file_ << position[0] << ' ' << position[1] << ' ' << position[2];
	end_line(file_, line_delimitor_);
	count_++;
}


void ply_exporter::write_full_binary_(const point_full& p) {
	Eigen_vec3 position = p.position();
	Eigen_vec3 normal = p.normal();
	rgb_color col = p.color();
	
	file_.write(reinterpret_cast<const char*>( position.data() ), 3 * sizeof(Eigen_scalar));
	file_.write(reinterpret_cast<const char*>( normal.data() ), 3 * sizeof(Eigen_scalar));
	file_.write(reinterpret_cast<const char*>( &col ), 3);
	count_++;
}


void ply_exporter::write_full_ascii_(const point_full& p) {
	Eigen_vec3 position = p.position();
	Eigen_vec3 normal = p.normal();
	rgb_color col = p.color();

	file_ << position[0] << ' ' << position[1] << ' ' << position[2]
		<< ' ' << normal[0] << ' ' << normal[1] << ' ' << normal[2]
		<< ' ' << (unsigned)col.r << ' ' << (unsigned)col.g << ' ' << (unsigned)col.b;
	end_line(file_, line_delimitor_);
	count_++;
}

}
