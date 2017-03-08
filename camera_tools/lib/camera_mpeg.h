#ifndef C3DLIC_CAMERA_MPEG_H_
#define C3DLIC_CAMERA_MPEG_H_

#include <iosfwd>

class camera;

bool read_camera_mpeg(std::istream& input, camera&, bool convert = true);
void write_camera_mpeg(std::ostream& output, const camera&, bool convert = true);


#endif