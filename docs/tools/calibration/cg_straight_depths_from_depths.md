# calibration/cg\_straight\_depths\_from\_depths

Estimate feature [straight depths](../../data/straight_depths.html) from aggregated feature point depth.

    calibration/cg_straight_depths_from_depths image_correspondences.json intrinsics.json R.json out_straight_depths.json

Feature point depths must have been put into the image correspondences first using [calibration/read\_feature\_depths](read_feature_depths.html). Rotation and intrinsics must be given. For each feature: calculates the straight depth for each view, and then averages them (filtering out outliers).

Reports average variance as an error metric.
