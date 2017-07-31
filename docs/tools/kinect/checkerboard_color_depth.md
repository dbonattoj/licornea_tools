# kinect/checkerboard\_color\_depth

Test depths error of checkerboard in color image, using Kinect reprojection calibration.

    kinect/checkerboard_color_depth cols rows square_width reprojection.json [out_stat.txt]
    
Detects checkerboard in _color_ image. For pixels in it, it takes the depth in two ways:

- Calculated using the checkerboard corners, the given `square_width` parameter, and the intrinsics of the color camera (part of `reprojection.json`).

- Measured by reprojecting the raw depth map using `reprojection.json`, and taking the values in the reprojected depth map.

Then compares these two values for each pixel. The reprojection depends on all calibrated values (color/IR intrinsics+distortion, pose transformation). So it tests if these are correct (when the error is small). 

Can also collect samples of the error depending on checkerboard position. Can be used to make a model of the absolute depth offset.
Unlike [kinect/checkerboard\_depth\_stat](tools/kinect/checkerboard_depth_stat.html), this is in _color image space_.
