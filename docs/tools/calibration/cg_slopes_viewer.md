# calibration/cg\_slopes\_viewer

Show model and measured optical flow slopes in GUI.

    calibration/cg_slopes_viewer dataset_parameters.json intrinsics.json points.json/measured_slopes.json

Displays are GUI where the modelled optical flow slopes for given [feature points](../../data/feature_points.html) are shown, with adjustable `(X,Y,Z)` rotation euler angles.  

[Feature slopes](../../data/feature_slopes.html) can also be given instead of feature points. Then it also displays those slopes. It can be used to manually fit the rotation such that the model matches the measurement.
