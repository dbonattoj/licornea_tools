# kinect/checkerboard\_depth\_stat

Saves statistics of calculated v. measured checkerboard depths in IR image.

    kinect/checkerboard_depth_stat chk_samples.json cols rows square_width ir_intr.json out_stat.txt

`chk_samples.json` is obtained from [kinect/checkerboard\_depth\_samples](checkerboard_depth_samples.html).

For all pixels on checkerboard (in IR image), takes their depth in two ways:

- Measured depth from raw depth map pixels.

- Calculated using detected checkerboard corners with `square_width`, using IR intrinsics.

They should be the same, unless IR intrinsics are wrong, but there is also an absolute offset. Tool takes samples of the difference depending on pixel position.
Can be used to make a plane/polynomial fitting model of the absolute offset.
