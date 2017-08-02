# Import dataset from Kinect

This is how to import an entire dataset (images and depths) from Kinect. It copies and renames the files, and reprojects and unsamples the depth maps.

## 1. Prepare dataset parameters

Make a [dataset parameters](../data/dataset.html) file `parameters.json` for the dataset. It should indicate the location of the input images and raw depth maps (not reprojected) from the Kinect, in the `kinect_raw`. And it should indicate the location of the final images and reprojected depth maps, in the root group.

The Kinect reprojection must have been [calibrated](calibrate_kinect.html) before. But the location of the [reprojection parameters](../data/reprojection.html) file `reprojection.json` into the `kinect_raw` group, under `kinect_reprojection_parameters_filename`.

If images are already at the correct location, put the same value into both `image_filename_format`. (Assuming there is no different numbering in `kinect_raw`)

## 2. Run the import script

To import all views, run

    kinect/import_raw_data.py parameters.json mine

Inside the `import_raw_data.py` source code, variables can be set to indicate whether to import only images or only depths, and whether to overwrite existing output files.

The depth maps will be reprojected using the given [reprojection parameters](../data/reprojection.html), and then upsampled with the given _densification method_ (here `mine`). This algorithm is implemented in `src/kinect/densify/depth_densify_mine.cc`.

If the [environment variable](../installation.html) `LICORNEA_PARALLEL` is set to `1`, it will run in parallel. It will also show estimated time remaining (But this can get wrong if many files are skipped). On Linux, due to a bug in **joblib** (?), it can sometimes block near the final views when running in parallel. Then, terminate it, remove incomplete output files, set it to not overwrite, and re-run it non-parallel, to complete the import.
