# calibration/evaluate\_calibration

Evaluate [camera parameters](../../data/cameras.html) by seeing if they make [image correspondences](../../data/image_correspondences.html) overlap.

    calibration/evaluate_calibration dataset_parameters.json cors.json cams.json out_samples.txt [random/all] [random_count=100000]

Must be given [image correspondences](../../data/image_correspondences.html) with feature point depths. Does not use the images or depth maps in the dataset.

For pairs of view indices `(ref,tg)`, warps feature points of `ref` to `tg`, using the extrinsic matrices of the two views, and the feature point depths of `ref`. Then measures the reprojection error, i.e. the root-mean square distance of the warped feature points and the corresponding feature points from `tg`. Assuming that the image correspondences are without error, then this error metric only measures the error in the [camera parameters](../../data/cameras.html).

Can either be run for each possible pair of views (option `all`). If the dataset if too large for this, can instead be set to sample randomly selected pairs of views (option `random`).

Outputs `out_samples.txt`, which contains, for each test, the measured reprojection error, and the _baseline_ of the view indices pair. The _baseline_ is the 2D euclidian distance between view indices.
