# kinect/calibrate\_color\_ir\_reprojection

Estimate pose transformation matrix from Kinect color camera to IR camera spaces.

    kinect/calibrate_color_ir_reprojection obj_img_cors_set.json color_intrinsics.json ir_intrinsics.json out_reprojection_params.json
        
`color_intrinsics.json` and `ir_intrinsics.json` are obtained from [calibration/calibrate\_intrinsics](../calibration/calibrate_intrinsics.html).

`obj_img_cors_set.json` is a 1-object-2-image [1-object-2-image correspondences set](../../data/obj_img_cors.html) with coordinates of the checkerboard corners on the color and IR image. It is obtained using [kinect/checkerboard\_samples](checkerboard_samples.html).

The program outputs the [reprojection parameters](../../data/reprojection.html) for the Kinect.
