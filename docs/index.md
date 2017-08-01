# Documentation

These are several programs that do single processing steps in the 3DLicorneA pipeline. Each program works independently, taking some input data and generating output data. Parameters are usually exchanged in JSON-based formats.

The programs are organized into thematic groups, in the different subdirectories `vsrs/`, `kinect/`, etc. Each `.py` and each `.cc` file is one program. Only the C++ programs operate on the content of images. Some Python programs are scripts that call other programs on a set of inputs, to do batch operations. (For example, exporting an entire dataset to VSRS format).

Building installs all the executables/scripts into the `bin/` directory, from which they should be used.

## Information

- [Installation](installation.md): How to build and install the tools
- [CG report](cg.pdf): Method and tutorial of camera grid calibration
- [Epipolar lines report](epipolar.pdf): How to compute epipolar lines from camera parameters


## Data formats

- [Dataset parameters](data/dataset.html)
- [Camera parameters](data/cameras.html)
- [VSRS experiments](data/experiments.html)
- [Feature points](data/feature_points.html)
- [Feature point slopes](data/feature_points.html)
- [Image correspondences](data/image_correspondences.html)
- [Camera intrinsics](data/intrinsics.html)
- [Object-image-correspondences](data/obj_img_cors.html)
- [Reference views grid](data/references_grid.html)
- [Relative camera positions](data/relative_camera_positions.html)
- [Kinect reprojection parameters](data/reprojection.html)
- [Straight depths](data/straight_depths.html)
- [View homographies](data/view_homography.html)

