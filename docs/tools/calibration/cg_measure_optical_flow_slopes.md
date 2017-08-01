# calibration/cg\_measure\_optical\_flow\_slopes

Estimate [slopes](../../data/feature_slopes.html) of feature points, for use with rotation estimation.

    calibration/cg_measure_optical_flow_slopes dataset_parameters.json image_correspondences.json intrinsics.json out_slopes.json

In the [image corresponcences](../../data/image_correspondences.html), takes for each feature, the feature points horizontally and vertically around the reference view, and uses line fitting to estimate the [slope](../../data/feature_slopes.html) of the lines that they form. Assumes the camera centers were aligned (approximately) on a orthogonal grid.

The [intrinsics](../../data/intrinsics.html) will be used to first undistort the image corresponcences, if required.

The output slopes can then be used to estimate the camera rotation, using [calibration/cg\_rotation\_from\_fslopes](cg_rotation_from_fslopes.html). They can also visualized with [calibration/cg\_slopes\_viewer](cg_slopes_viewer.html).
