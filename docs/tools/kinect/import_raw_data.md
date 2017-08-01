# kinect/import\_raw\_data

Batch copy images and reproject depth maps from Kinect.

    kinect/import_raw_data.py dataset_parameters.json densify_method [simulate]

In the given [dataset parameters](../../data/dataset.html), the textures and depth filenames must be defined for the root group, and for the `kinect_raw` group.

It copies textures from `kinect_raw` files to the files for the root group.

And it reprojects the raw depths from `kinect_raw`, into the depth files in root group. Reprojection is done using [kinect/depth\_reprojection](depth_reprojection.html), with the given `densify_method`.

Parameters in the script can be set to define if only textures or only depths should be processed, and if existing files should be overwritten.

If `simulate` is set, no files are written, only output is printed.

The batch script can run parallelized.
