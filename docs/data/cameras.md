# Camera parameters

Similar to the [dataset parameters](dataset.html), _camera parameters_ are stored in a JSON format used by all the tools.
Each camera is defined by a _name_, an _extrinsic_ matrix and an _intrinsic_ matrix.

The extrinsic matrix `Rt` is a 4x4 rigid transformation matrix in homogeneous coordinates. The last row is always
`0 0 0 1`. For a point `w = (X,Y,Z,1)` in _world space_, `v = Rt * w` results in the same point
in _view space_ of this camera.

The tool [camera/export_mpeg](../tools/camera/export_mpeg.html) exports the JSON file to a MPEG-format camera file usable with VSRS and DERS, and automatically
does the necessary conversion of the extrinsic matrices. [camera/import_mpeg](../tools/camera/import_mpeg.html) imports a MPEG-format camera file into
a JSON file, and does the inverse conversion.
