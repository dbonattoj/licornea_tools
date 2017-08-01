# calibration/cameras\_from\_checkerboards

Estimate camera poses of dataset, using checkerboard detected in all images.

    calibration/cameras_from_checkerboards dataset_parameters.json cols rows square_width intr.json out_cameras.json [dataset_group]

Detects a checkerboard with given dimension and square width in each image of the dataset, estimates camera pose for it, and adds camera for it. [Intrinsics](../../data/intrinsics.html) of the dataset must be given. Output are [camera parameters](../../data/cameras.html).

Skips views where checkerboard cannot be detected or image file is missing.

Needs only images, no depth maps.
