# kinect/remapping\_viewer

Compare calibrated IR to color remapping to internal parameters.

    kinect/remapping_viewer internal.json reprojection.json

Uses both the calibrated [reprojection parameters](../../data/reprojection.html), and the Kinect's internal parameters, to remap pixels from the IR image onto the color image.

Measures the error between the resulting pixel position for each raw IR pixel. This tests of the calibrated reprojection parameters are correct.
