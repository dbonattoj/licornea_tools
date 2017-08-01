# calibration/read\_feature\_depths

Read feature point depths into of [image correspondences](../../data/image_correspondences.html), from the depth maps.

    calibration/read_feature_depths dataset_parameters.json in_cors.json out_cors.json [xy_outreach=0] [step=1]

Opens each depth map file once, and reads the depth of each feature point of the given [image correspondences](../../data/image_correspondences.html) on it. `out_cors.json` will be a copy of `in_cors.json`, with the feature point depths added to it. `in_cors.json` and `out_cors.json` can also be set to the same.

If `xy_outreach` is non-zero, it does not just take one pixel of the depth map, but instead takes a small rectangular value around the feature point (reaching out `xy_outreach` pixels), and chooses the minimal depth value in it. Tracked feature points are often on the border of foreground objects. In the (upscaled) depth map, the same pixel can sometimes fall on the background object. This insures that always the depth of the foreground object is chosen.

