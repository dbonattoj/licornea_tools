# calibration/undistort\_cors

Undistort feature points of [image correspondences](../../data/image_correspondences.html).

    calibration/undistort_cors in_cors.json out_cors.json intrinsics.json

In case image correspondences were taken on distorted images, this removes the distortion from the image correspondences. [Intrinsics](../../data/intrinsics.html) with the distortion coefficients must be given.
