# camera/transform

Apply different transformations to camera parameters.

    camera/transform transform in_cameras.json out_cameras.json/none/replace operation

Run without arguments to see possible `operations`.  If output file is `none` it does not write output. If it is `replace`, it replaces to input file.

The `MPEG2Rt` and `Rt2MPEG` transformations should never be necessary, as it is done automatically by [camera/export\_mpeg](export_mpeg.html) and [camera/import\_mpeg](import_mpeg.html).
