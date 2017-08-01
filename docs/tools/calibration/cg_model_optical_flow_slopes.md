calibration/cg\_model\_optical\_flow\_slopes

Model [slopes](../../data/feature_slopes.html) of feature points with given rotation.

    calibration/cg_model_optical_flow_slopes feature_points.json intrinsics.json rotation.json out_predicted_slopes.json

Models optical flow [slopes](../../data/feature_slopes.html) that the given [feature points](../../data/feature_points.html) would form with the given camera rotation. Using this is not necessary, [calibration/cg_rotation_from_fslopes](cg\_rotation\_from\_fslopes.html) does it on its own.
