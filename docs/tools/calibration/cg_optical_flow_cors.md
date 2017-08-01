# calibration/cg\_optical\_flow\_cors

Computes the optical flow [image correspondences](../../data/image_correspondences.html), starting from chosen [feature points](../../data/feature_points.html) on reference view.

     calibration/cg_optical_flow_cors dataset_parameters.json reference_fpoints.json horiz_outreach vert_outreach out_cors.json [dataset_group]

Dataset can be 1D or 2D. For 2D, first moves in vertical direction form the reference view, then from each vertical position, to the horizontal. From the (center) reference view, goes up/down, and left/right.

Skips views with missing images. In horizontal direction, this means one view is skipped only. In vertical direction, it means the whole line is skipped. Therefore [calibration/cg\_choose\_refgrid](cg_choose_refgrid.html) chooses the reference views so that this does not happen.

Initial feature points on reference view should be chosen using [calibration/cg\_optical\_flow\_features](cg_optical_flow_features.html). If there are multiple reference views, this program must be run for each one individually.

It can take a long time, and produce a large output `out_cors.json`. For the output [image correspondences](../../data/image_correspondences.html), a file name extension `out_cors.bin` can be used instead. It writes them in a binary format that takes up less space.

Several parameters can be set in the `cg_optical_flow_cors.cc` source code. Including whether multi-scale pyramids should be constructed for the images.
