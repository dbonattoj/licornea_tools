# calibration/cg\_cors\_viewer\_f

Visualize feature points for one feature.

    calibration/cg_cors_viewer_f dataset_parameters.json cors.json closeup? [dataset_group]
    
Shows GUI where feature can be selected. Takes [image correspondences](../../data/image_correspondences.html) as input.

Displays feature points of that feature only, for all the different views overlaid. Backdrop view can also be selected.

Can also show depth map of that view. To see if the feature points also always lays on the correct depth map region.

If `closeup` is set, zooms in to only the region where the feature points are.

