# camera/import\_mpeg

Import camera parameters from MPEG format.

    camera/import_mpeg in_cameras_mpeg.txt out_cameras.json [no_convert]

`out_cameras.json` will be [camera parameters](data/cameras.json). It changes the convention of the **Rt** matrix. Unless `no_convert` is set (should never be necessary).
