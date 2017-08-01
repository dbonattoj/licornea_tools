# calibration/cg\_rotation\_from\_fslopes

Estimates camera rotation using optical flow [feature slopes](../../data/feature_slopes.html).

    calibration/cg_rotation_from_fslopes measured_feature_slopes.json intrinsics.json out_rotation.json

The [feature slopes](../../data/feature_slopes.html) must first have been measured using [calibration/cg\_measure\_optical\_flow\_slopes](cg_measure_optical_flow_slopes.html).  

It uses an iterative process to find a 3D rotation for which the modelled slopes well match the measured slopes.

