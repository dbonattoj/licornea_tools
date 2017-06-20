#ifndef LICORNEA_ROTATION_H_
#define LICORNEA_ROTATION_H_

#include "common.h"

namespace tlz {
	
bool is_orthogonal_matrix(const mat33& R);
mat33 to_rotation_matrix(const vec3& euler);
vec3 to_euler(const mat33& R);

}

#endif
