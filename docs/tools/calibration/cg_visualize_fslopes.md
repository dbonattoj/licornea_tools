# calibration/cg\_visualize\_fslopes

Make visualization of the [feature slopes](../../data/feature_slopes.html).

    calibration/cg_visualize_fslopes dataset_parameters.json slopes.json out_visualization.png [width=200] [exaggeration=1] [dataset_group]

Like [calibration/cg\_slopes\_viewer](cg_slopes_viewer.html), but saves the visualization in a PNG file instead of showing GUI. Length (`width`) of shown line segments for the horizontal and vertical slopes on each feature point, and `exxageration` factor of slopes can be set. Background is the view on which these [feature slopes](../../data/feature_slopes.html) are taken.
