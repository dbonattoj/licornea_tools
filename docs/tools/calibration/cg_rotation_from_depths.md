# calibration/cg\_rotation\_from\_depths

Estimate camera rotation using feature point depths in [image correspondences](../../data/image_correspondences.html).

    calibration/cg_rotation_from_depths cors.json intrinsics.json out_rotation.json

Feature point depth must have been read into the image correspondences, using [calibration/read\_feature\_depths](read_feature_depths.html).

It estimates the yaw and pitch rotation by with plane fitting for each feature. Then it tries to estimate the roll rotation such that the feature points become horizontally/vertically aligned.
