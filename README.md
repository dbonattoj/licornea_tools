# licornea_tools
Tools for 3DLicorneA
* `pointcloud/`: Generate XYZRGB `.ply` point cloud from VSRS config file and `.yuv` texture/disparity data.
* `camera_tools/transform`: Operations on extrinsic/intrinsic camera matrices, in VSRS/DERS camera array format.
* `camera_tools/visualize`: Visualize camera poses from camera array. Outputs `.ply` file with 3D representations for cameras.
* `camera_tools/parameters_xml2vsrs.sh`: Convert XML camera parameters file to VSRS/DERS camera array format.
* `epipolar_sketch/`: Visualizer for epipolar lines and corresponding points using camera matrices. Sketch written in p5.js.
* `kinect_tools/src/depth_point_cloud`: Reproject Kinect depth map into XYZ `.ply` point cloud, in camera view coordinate system.
* `kinect_tools/src/depth_reprojection`: Reproject&upsample Kinect depth map into same coordinate system as color image.
* `kinect_tools/src/fetch_intrinsics`: Fetch internal camera parameters from Kinect connected by USB, store in JSON file.
* `kinect_tools/src/texture_reprojection`: Reproject&downsample Kinect color image into same coordinate system as depth image.
* `kinect_tools/src/vsrs_disparity`: Convert reprojected Kinect depth map into disparity map for VSRS in `.yuv` format.
* `kinect_tools/make_vsrs_data.py`: Make VSRS texture+disparity `.yuv` files, for all views of data sets, from Kinect depth+image, with either depth upsampling or texture downsampling, using the other tools.

Online version of epipolar sketch:
https://timlenertz.github.io/licornea_tools/epipolar_sketch/
