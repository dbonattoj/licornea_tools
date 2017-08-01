# calibration/undistort\_fpoints

Undistort [feature points](../../data/feature_points.html).

    calibration/undistort_fpoints in_fpoints.json out_fpoints.json intrinsics.json

In case image correspondences were taken on distorted images, this removes the distortion from one set of [feature points](../../data/feature_points.html). [Intrinsics](../../data/intrinsics.html) with the distortion coefficients must be given.

To undistort all feature points in [image correspondences](../../data/image_correspondences.html), use [calibration/undist\_cors](undist_cors.html) instead.
