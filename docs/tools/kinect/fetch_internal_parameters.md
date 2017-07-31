# kinect/fetch\_internal\_parameters

Takes the internal parameters that are stored inside the connected Kinect, and save them.

    kinect/fetch_internal_parameters out_internal_parameters.json

These internal parameters are intrinsics of the color and IR camera (but better ones can be calibrated). And a non-linear model for the remapping, that does not take the change in absolute depth into account.

They are different in each Kinect unit, and using the wrong ones gives incorrect results.
