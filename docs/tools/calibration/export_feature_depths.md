# calibration/export\_feature\_depths

Export the feature point depths of one line of views.

    calibration/export_feature_depth dataset_parameters.json in_cors.json depths.txt [y_index]

Given [image correspondences](../../data/image_correspondences.html) must have feature point depths. If dataset if 2D, `y`-index of line `y_index` must be. Output `depths.txt` will contain the feature point depth for each feature, for each `x` index.

Can be used to see if the feature point depths are correct, i.e. the scene object whose depth was taken on the depth map, remains the same, for all the views.
