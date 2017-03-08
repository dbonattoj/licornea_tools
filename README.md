# Tools for 3DLicorneA

## Camera tools
* `export_mpeg`: Convert JSON format to DERS/VSRS camera file, having MPEG matrix convention.
* `import_mpeg`: Convert DERS/VSRS camera file, having MPEG matrix convention to JSON format.
* `import_xml`: Convert XML camera file to JSON format.
* `transform`: Operations on extrinsic/intrinsic camera matrices.
* `visualize`: Visualize camera poses from camera array. Outputs `.ply` file with 3D representations for cameras.

## Kinect tools
* `depth_point_cloud`: Reproject Kinect depth map into XYZ `.ply` point cloud, in camera view coordinate system.
* `depth_reprojection`: Reproject&upsample Kinect depth map into same coordinate system as color image.
* `fetch_intrinsics`: Fetch internal camera parameters from Kinect connected by USB, store in JSON file.
* `texture_reprojection`: Reproject&downsample Kinect color image into same coordinate system as depth image.
* `vsrs_disparity`: Convert reprojected Kinect depth map into disparity map for VSRS in `.yuv` format.
* `make_vsrs_data`: Make VSRS texture+disparity `.yuv` files, for all views of data sets, from Kinect depth+image, with either depth upsampling or texture downsampling, using the other tools.

## Misc
* `pointcloud`: Generate XYZRGB `.ply` point cloud from VSRS config file and `.yuv` texture/disparity data.

## Epipolar sketch
* `epipolar_sketch`: Visualizer for epipolar lines and corresponding points using camera matrices. Sketch written in p5.js.

Online version:
https://timlenertz.github.io/licornea_tools/epipolar_sketch/
