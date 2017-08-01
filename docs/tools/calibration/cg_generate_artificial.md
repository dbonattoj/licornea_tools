# calibration/cg\_generate\_artificial

Generate artificial dataset with images and depth maps, with randomized known features.

    calibration/cg_generate_artificial R.json intr.json out_datas_dir/ [features_count=100] [num_x=30] [num_y=30] [step_x=3.0] [step_y=3.0]

Camera rotation matrix `R.json`, and camera intrinsics `intr.json` for the new dataset must be given as input. Files for dataset will be written into `out_datas_dir/`, consisting of:

- `out_datas_dir/parameters.json`: [Dataset parameters](../../data/dataset.html)
- `out_datas_dir/cors.json`: Perfect [image correspondences](../../data/image_correspondences.html) from simulated optical flow.
- `out_datas_dir/straight_depths.json`: Perfect [straight depths](../../data/straight_depths.html) for the features.
- `out_datas_dir/cameras.json`: Perfect [camera parameters](../../data/cameras.html) of the dataset;
- `out_datas_dir/image/y###_x###.png`: Generated textures of the dataset.
- `out_datas_dir/depth/y###_x###.png`: Generated depth maps of the dataset, 16 bit monochrome PNG files.

The scene consists of colored dots at different distances to the camera, randomly distributed in space, with disks around them in the textures. The generated images are not suited to do an optical flow on them, however.

`num_x`, `num_y` is the number of views in horizontal and vertical direction. `step_x` and `step_y` the spacial distance between adjacent cameras. `features_count` the count of features.

Optionally, some random noise can be added to the camera center positions, and/or to the camera's orientations. This can be enabled inside the `cg_generate_artificial.cc` file. The noise on the camera center positions is gaussian, and some views are randomly chosen as outliers, and given a larger noise.

The purpose is to have a dataset where the true correspondences, straight depth, rotation, and camera poses are known, so that the results of the other processes can be evaluated. It also gives a dataset without all the errors/noise from a real dataset.

A `R.json` file can be obtained with [misc/cam\_rotation](../misc/cam_rotation.html), the `intr.json` can easily be manually made.

