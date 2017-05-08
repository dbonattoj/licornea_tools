#ifndef LICORNEA_MULTIPROJECTION_H_
#define LICORNEA_MULTIPROJECTION_H_

#include "../../lib/json.h"
#include <vector>

namespace tlz {

class multiprojection {
public:
	struct input {
		std::string point_cloud_filename;
		std::string camera_name;
	};
	std::string out_camera_name;
	std::vector<input> inputs;
};

multiprojection decode_multiprojection(const json&);
json encode_multiprojection(const multiprojection&); 

}

#endif
