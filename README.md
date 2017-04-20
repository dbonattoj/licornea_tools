# Tools for 3DLicorneA

## Calibration
...

## Camera
* `export_mpeg`: Convert JSON format to DERS/VSRS camera file, having MPEG matrix convention.
* `import_mpeg`: Convert DERS/VSRS camera file, having MPEG matrix convention to JSON format.
* `import_xml`: Convert XML camera file to JSON format.
* `transform`: Operations on extrinsic/intrinsic camera matrices.
* `visualize`: Visualize camera poses from camera array. Outputs `.ply` file with 3D representations for cameras.

## Kinect
* `collect_col`: Collect all images on one column, from raw data set.
* `depth_point_cloud`: Reproject Kinect depth map into XYZ `.ply` point cloud, in color or IR camera view coordinate system.
* `depth_reprojection`: Reproject&upsample Kinect depth map into same coordinate system as color image.
* `fetch_intrinsics`: Fetch internal camera parameters from Kinect connected by USB, store in JSON file.
* `vsrs_disparity`: Convert reprojected Kinect depth map into disparity map for VSRS in `.yuv` format.
* `make_vsrs_data`: Make VSRS texture+disparity `.yuv` files, for all views of data sets, from Kinect depth+image, with either depth upsampling or texture downsampling.

## Misc
* `list_skin_n_experiments`: List synthesis experiments for each view, taking each _k_-th view as input.
* `make_vsrs_config`: Generate VSRS configuration file based on template in script.
* `run_vsrs`: Generate temporary VSRS configuration file and run VSRS on given input set and parameters.
* `run_vsrs_experiments`: Run VSRS for all listed experiments, parallelized.
* `psnr`: Compute PSNR between two images.
* `yuv2png`: Convert YUV420 (with given size) to image file.
* `view_depth`: Visualize depth map with GUI.

## Epipolar sketch
* `epipolar_sketch`: Visualizer for epipolar lines and corresponding points using camera matrices. Sketch written in p5.js.

Online version:
https://timlenertz.github.io/licornea_tools/epipolar_sketch/
