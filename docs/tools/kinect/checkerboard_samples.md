# kinect/checkerboard\_samples

Take samples of checkerboard corners, in color and/or IR image.

    kinect/checkerboard_samples color/ir/both cols rows square_width out_cors_set.json [out_images_dir/] [restore_from_images]

If first parameter is `color` or `ir`, detects the checkerboard in that image only. Hitting _Enter_ saves the checkerboard corners. `out_cors_set.json` is a [1-object-1-image correspondences set](data/cat_obj_img_cors.html), where object space is the checkerboard.

If first parameter is `both`, detects the checkerboard in both at the same time. Then `out_cors_set.json` is a [1-object-2-image correspondences set](data/cat_obj_img_cors.html). Then the checkerboard must not be moving when taking the sample, otherwise it will be wrong: there is a time delay between color and IR image.

If `out_images_dir/` is given, also saves the images in that directory.

Normally it displays a GUI and connects to the Kinect. But if `restore_from_images` is set, it detects the checkerboards in the saved images in `out_images_dir/`, writes it to `out_cors_set.json` and then exits.
