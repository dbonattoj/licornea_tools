# calibration/cg\_rcpos\_from\_cors

Estimate [relative camera positions](../../data/relative_camera_positions.html)

    calibration/cg_rcpos_from_cors dataset_parameters.json cors.json intr.json R.json straight_depths.json out_rcpos.json [out_sample_positions.txt] [out_final_positions.txt]

Given the [image correspondences](../../data/image_correspondences.html) (possibly merged with different reference views), the [intrinsics](../../data/intrinsics.html), the estimates camera rotation, and the estimated [straight depths](../../data/straight_depths.html), estimate [relative camera positions](../../data/relative_camera_positions.html).

Optionally outputs all the camera position samples `c_f,v`, as well as the final camera positions `c_v`.

Parameters can be adjusted in the `cg_rcpos_from_cors.cc` source code, such as whether to find and remove bad features (because of bad straight depth) first.

