# Documentation

These are several programs that do single processing steps in the 3DLicorneA pipeline. Each program works independently, taking some input data and generating output data. Parameters are usually exchanged in JSON-based formats.

The programs are organized into thematic groups, in the different subdirectories `vsrs/`, `kinect/`, etc. Each `.py` and each `.cc` file is one program. Only the C++ programs operate on the content of images. Some Python programs are scripts that call other programs on a set of inputs, to do batch operations. (For example, exporting an entire dataset to VSRS format).

Building installs all the executables/scripts into the `bin/` directory, from which they should be used.

## Information

- [Installation](installation.md) - How to build and install the tools
- [CG report](cg.pdf) - Method and tutorial of camera grid calibration
- [Epipolar lines report](epipolar.pdf) - How to compute epipolar lines from camera parameters
- [Epipolar sketch](epipolar_sketch/index.html) - Interactive sketch showing epipolar lines


## Data formats

- [Dataset parameters](data/dataset.html) - Indexing and file names of dataset
- [Camera parameters](data/cameras.html) - Array of intrinsic and extrinsic camera parameters
- [Image correspondences](data/image_correspondences.html) - Corresponding feature points on many views
- [Feature points](data/feature_points.html) - Set of points on a views
- [Feature point slopes](data/feature_points.html) - Slope of the line that feature points form
- [Straight depths](data/straight_depths.html) - Planar depths of tracked features
- [Reference views grid](data/references_grid.html) - Grid of reference view indices
- [Relative camera positions](data/relative_camera_positions.html) - Positions of camera centers, relative to reference view
- [Camera intrinsics](data/intrinsics.html) - Camera matrix and distortion coefficients
- [Object-image-correspondences](data/obj_img_cors.html) - Corresponding points between 2D images and 3D objects
- [Kinect reprojection parameters](data/reprojection.html) - Parameters to map Kinect depth pixels to color pixels
- [VSRS experiments](data/experiments.html) - List of view synthesis experiments
- [View homographies](data/view_homography.html) - Homography matrices to apply to images

