# kinect/color\_intrinsic\_reprojection

Measure and visualize the reprojection error of the Kinect color intrinsics.

    kinect/color_intrinsic_reprojection cols rows square_width color_intr.json

Detects checkerboard in Kinect color camera. Shows detected corners, and the same corners after they were projected from world space back onto the image, using the given intrinsics.
