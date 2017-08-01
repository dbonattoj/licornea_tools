# calibration/calibrate\_intrinsics

Estimate camera intrinsics from a [1-object-1-image correspondences set](../../data/obj_img_cors.html).

    calibration/calibrate_intrinsics obj_img_cors_set.json image_width image_height out_intrinsics.json [no_distortion]

Can for example use checkerboard detected using [kinect/checkerboard\_samples](../kinect/checkerboard_samples.html). Width and height of the images on which the correspondences are, must be given. Output `out_intrinsics` is an [intrinsics](../../data/intrinsics.html) file. If `no_distortion` is set, assumes zero distortion, otherwise also optimizes distortion coefficients.
