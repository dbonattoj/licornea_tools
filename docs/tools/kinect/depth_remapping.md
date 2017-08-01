# kinect/depth\_remapping

Remap raw Kinect depth map to color image.

    kinect/depth_remapping input.png output.png output_mask.png internal_parameters.json method [=was_flipped/was_not_flipped]

Similar to [kinect/depth\_reprojection](depth_reprojection.html), but it does not used the calibrated reprojection parameters, and instead only the Kinect's internal parameters. They are extracted from the Kinect using [kinect/fetch\_internal\_parameters](fetch_internal_parameters.html).

The algorithm is taken Freenect2, and that one was reverse-engineered from the Microsoft SDK. It only moves around depth pixels without changing their values, it does not take into account that the orthogonal depths are slightly different from the point of view of the color sensor.

[kinect/depth\_reprojection](depth_reprojection.html) should be used instead.
