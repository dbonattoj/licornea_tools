# calibration/cg\_filter\_features

Automatically filter bad features from [image correspondences](../../data/image_correspondences.html).

    calibration/cg_filter_features dataset_parameters.json cors.json out_cors.json expected_x_count expected_y_count [use_depth]

Arguments `expected_x_count` and `expected_y_count` are how much points there should be in horizontal and vertical directions. Should be about the _outreach_ from the [calibration/cg\_optical\_flow\_cors](calibration/cg_optical_flow_cors.html), divided by two. (If _reference view_ is near the limits of dataset, outreach is done in one direction only.)

If `use_depth` is set, also removed features where feature points depth is not constant enough. If should vary slightly because of the camera's rotation and because of noise, but there should not be big jumps. This would indicate that it jumps to the depth of another (background) object, because of how the borders were drawn on the (upscaled) depth map. Then averaging them for the computation of the [straight depths](../../data/straight_depths.html) will not give good results. The evolution of the depth can be seen using [calibration/export\_feature\_depths](export_feature_depths.html).

The filtering criteria are:

- At least `expected_x_count` feature points on middle horizontal line
- At least `expected_y_count` feature points on middle vertical column
- Total number of feature points v. total number of views
- Estimate lattice, how well do feature points match lattice
- Constancy of feature point depth (if enabled)

There are several parameters that should be adjusted, at the beginning inside the `cg_filter_features.cc` file.

Manual filtering should always also be done, using [calibration/cg\_cors\_viewer\_f](cg_cors_viewer_f.html) to find bad feature, and then [calibration/remove\_cors](remove_cors.html) to remove them.
