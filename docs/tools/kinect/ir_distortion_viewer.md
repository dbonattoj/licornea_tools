# kinect/ir\_distortion\_viewer

Views the effect of IR intrinsic distortion.

    kinect/ir_distortion_viewer ir_intrinsics.json/internal ir/depth

Shows the IR image or depth map from Kinect, before and after the undistortion.

Either an [IR intrinsics](../../data/intrinsics.html) file is given, or with `internal` the IR intrinsic parameters stored in the Kinect are fetched and used.
