# kinect/ir\_intrinsic\_reprojection

Measure and visualize the reprojection error of the Kinect IR intrinsics.

    kinect/ir_intrinsic_reprojection cols rows square_width ir_intr.json

Detects checkerboard in Kinect IR camera. Shows detected corners, and the same corners after they were projected from world space back onto the image, using the given intrinsics.
