# calibration/cg\_stitch\_cameras

Stitch [relative camera positions](../../data/relative_camera_positions.html), giving final [camera parameters](../../cameras.json).

    calibration/cg_stitch_cameras dataset_parameters.json refgrid.json rcpos.json intr.json R.json out_cameras.json [out_camera_centers.txt]

Stitches [relative camera positions](../../data/relative_camera_positions.html) optained from [calibration/cg\_rcpos\_from\_cors](calibration/cg_rcpos_from_cors.html). Optionally the camera optical center positions can also be outputted into `out_camera_centers.txt`. It also contains (marked) the ones that were not selected.
