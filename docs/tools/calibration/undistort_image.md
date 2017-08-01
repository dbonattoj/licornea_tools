# calibration/undistort\_image

Undistort an image or a depth map.

    calibration/undistort_image in_image.png out_image.json intrinsics.json texture/depth
    
Uses the distortion coefficients in the given [intrinsics](../../data/intrinsics.html) to remove the distortion in the given texture or depth map.

If fourth argument is `texture`, 3-channel image is expected, and bicubic interpolation is used. If it is `depth`, 16-bit monochrome image is expected, and nearest-neighbor interpolation is used, so as to not introduce additional, incorrect depth values.
