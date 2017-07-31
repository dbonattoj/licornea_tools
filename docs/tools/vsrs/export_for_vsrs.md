# vsrs/export\_for\_vsrs

Export a complete dataset to VSRS format, generating YUV images, and YUV disparity maps.
      
    vsrs/export_for_vsrs.py dataset_parameters.json [simulate] [dataset_group]

`dataset_parameters.json` is a [dataset parameters](data/dataset.html) file for a dataset. If `simulate` is set, nothing is written, and only a simulation run is made. `dataset_group` can be set to a group of the dataset. Otherwise the default group is used.

Inside the script, variables can be set to indicate whether images and/or depth should be generated (`image`, `depth`), and if they should be overwritten if they already exist (`overwrite_image`, `overwrite_depth`).

The [dataset parameters](data/dataset.html) must contain `image_filename_format` and `depth_filename_format` for the `vsrs` group, and for the input (default or `dataset_group`) group. If must also contain `z_far` and `z_near` in the `vsrs` group.

The program will for each view index in the dataset (1D or 2D), read the input image and depth files, and write YUV image and disparity files into the path from the `vsrs` group.

Depth maps are converted to YUV disparity maps using [vsrs/vsrs\_disparity](tools/vsrs/vsrs_disparity.html), with the `z_far` and `z_near` values from the dataset parameters. Images are converted from the original files to YUV420 files using [tools/misc/yuv\_export](tools/misc/yuv_export.html). The batch process can be parallelized.
