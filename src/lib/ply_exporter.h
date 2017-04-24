#ifndef LICORNEA_PLY_EXPORTER_H_
#define LICORNEA_PLY_EXPORTER_H_

#include <fstream>
#include <string>
#include "point.h"
#include "utility/io.h"
#include "nd.h"

namespace tlz {

/// Exports point cloud into PLY file.
/** Can generate binary or ASCII format, and either only XYZ coordinates or with color, weight and normals. */
class ply_exporter {
private:
	std::ofstream file_;
	std::ofstream::pos_type vertex_count_string_position_;
	std::size_t count_ = 0;
	
	const line_delimitor line_delimitor_;
	bool full_;
	bool ascii_;
		
	void write_line_(const std::string& ln);
	
	void write_binary_(const point_xyz&);
	void write_ascii_(const point_xyz&);
	void write_full_binary_(const point_full&);
	void write_full_ascii_(const point_full&);


public:
	explicit ply_exporter(
		const std::string& filename,
		bool full = true,
		bool ascii = false,
		line_delimitor ld = line_delimitor::LF
	);
	~ply_exporter();
	
	template<typename It> void write(It begin, It end);
	
	void close();
};

}

#include "ply_exporter.tcc"

#endif
