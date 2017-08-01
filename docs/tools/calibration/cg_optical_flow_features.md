# calibration/cg\_optical\_flow\_features

Chooses [feature points](../../data/feature_points.html) suitable to compute optical flow [image correspondences](../../data/image_correspondences.html).

    calibration/cg_optical_flow_features dataset_parameters.json [refgrid.json] [out_fpoints_dir/] [dataset_group]

Displays a GUI where parameters can be adjusted, and the chosen feature points are shown. The parameters from [`cv::goodFeaturesToTrack`](http://docs.opencv.org/2.4/modules/imgproc/doc/feature_detection.html#goodfeaturestotrack) can be adjusted. Also, to well-distribute features in the image, it can first subdivide the image into `n` rectangles, select features on each one, and keep all the best features.

If a [references grid](../../data/references_grid.html) `refgrid.json` is given, chooses the feature points on all the views of the references grid. Hitting _Enter_ generates a [feature points](../../data/feature_points.html) file for each reference view in the directory `out_fpoints_dir/`. If the references grid is not give, one view can be freely selected.

The features will have unique names like `feat_RFFF`, where `R` is the number of the reference view, and `FFF` a number of the feature.

The optical flow should then be computed using [calibration/cg\_optical\_flow\_cors](cg_optical_flow_cors.html), once for each of the [feature points](../../data/feature_points.html) files.
