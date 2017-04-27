#ifndef LICORNEA_CAMERA_MPEG_H_
#define LICORNEA_CAMERA_MPEG_H_

#include <iosfwd>

namespace tlz {

struct camera;

bool read_camera_mpeg(std::istream& input, camera&, bool convert = true);
void write_camera_mpeg(std::ostream& output, const camera&, bool convert = true);

}

#endif
