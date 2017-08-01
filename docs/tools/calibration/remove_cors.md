# calibration/remove\_cors

Remove some features from [image correspondences](../../data/image_correspondences.html).

    calibration/remove_cors in_cors.json out_cors.json removed_feat1,removed,feat2,...
    
Removes all of the feature points for the given features from `in_cors.json`, and writes the result into `out_cors.json`. The third argument are the names of the features to remove, separated by `,`. There must be no space between the names, only `,`.
