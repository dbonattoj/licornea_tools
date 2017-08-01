# calibration/cors\_info

Displays information about [image correspondences](../../data/image_correspondences.html).

    calibration/cors_info dataset_parameters.json cors.json

Displays following information about to [image correspondences](../../data/image_correspondences.html) file:

- Reference views
- Total number of features (that occur in at least one view)
- Dataset group (if set)
- For each reference view, the features for that reference view, with the number of views on which it occurs
- Histogram of number of features per view; number of views with few features

