# calibration/cg\_straight\_depths\_from\_disparity

Estimate feature [straight depths](../../data/straight_depths.html) from relative scales of feature points.

    calibration/cg_straight_depths_from_disparity image_correspondences.json intrinsics.json R.json straight_depths.json out_straight_depths.json

Rotation and intrinsics must be given. But the [image correspondences](../../data/image_correspondences.html) need not have feature point depths. Some (can be only one) straight depths need to be given as input. It/they will only used to determine the scale factor at the end (which is important, because it determined the scale of the entire camera parameters in the end). Giving more than one, only serves to average away the error in those straight depths.

First calculates the relative scale of the feature points for each pair of features. Removes those that give poor (erronous) results. Then aggregates them into global scales, fixing one of them to `1`. Finally multiplies them to the correct scale using the straight depths given as input.

Parameters can be set in source code `cg_straight_depths_from_disparity.cc`, including whether to make a visualization PNG file of the pairwise scales matrix.
