#ifndef LICORNEA_STRAIGHT_DEPTHS_H_
#define LICORNEA_STRAIGHT_DEPTHS_H_

#include "../../../lib/common.h"
#include "../../../lib/json.h"
#include "../../../lib/args.h"

namespace tlz {

struct straight_depth {
	real depth;
	real confidence;
	
	straight_depth(real d = NAN, real conf = 1.0) : depth(d), confidence(conf) { }
	operator real () const { return depth; }
};

using straight_depths = std::map<std::string, straight_depth>;

json encode_straight_depths(const straight_depths&);
straight_depths decode_straight_depths(const json&);

straight_depths straight_depths_arg();
	
}

#endif
