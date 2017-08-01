# Kinect reprojection parameters

Parameters needed to reproject Kinect v2 raw depth map (or IR image), onto color image.

Consists of:

- [Intrinsics](intrinsics.html) of color camera (has no distortion)
- [Intrinsics](intrinsics.html) of IR/depth camera (with distortion)
- Rigid transformation matrix (rotation and translation) from color camera space to IR camera space
- _Optional_: 2D polynomial model of absolute depth offset in IR camera space
- _Optional_: 2D polynomial model of absolute depth offset in reprojected color camera space

