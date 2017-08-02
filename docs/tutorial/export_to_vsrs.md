# Export dataset to VSRS

This is how to export an entire dataset (images and depths) to VSRS. It must first have been _imported_ (for example, [from Kinect](import_from_kinect.html)), and contain the reprojected depth maps.


## 1. Prepare dataset parameters

In the [dataset parameters](../data/dataset.html), the `image_filename_template` and `depth_filename_template` in the root group must point to existing PNG images and reprojected depth maps. Add a `vsrs` group, and put in the file names of the YUV files for images and disparity maps, that should be generated.

Also inside the `vsrs` group, add two numbers `z_near` and `z_far`. They should indicate the minimal and maximal depth values in the depth maps, and are used for the convertion from depth map to disparity map. (See [vsrs/vsrs\_disparity](../tools/vsrs/vsrs_disparity.html)). Good values can be found by using [misc/view\_depth](../tools/misc/view_depth.html) on one of the depth maps.


## 2. Run the export script

Then, run

    vsrs/export_for_vsrs.py parameters.json

If the [environment variable](../installation.html) `LICORNEA_PARALLEL` is set to `1`, it will run in parallel, like with the [import from Kinect](import_from_kinect.html) script.


## 3. Export camera parameters

To convert the [camera parameters](../data/cameras.html) file to VSRS format, use

    camera/export_mpeg cameras.json cameras_vsrs.txt

