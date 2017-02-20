# licornea_tools
Tools for 3DLicorneA
* `texture2yuv.sh`: Convert .png to .yuv for VSRS, using `ffmpeg`
* `batch_texture2yuv.sh`: Apply `texture2yuv.sh` for all files in a directory.
* `parameters_xml2vsrs.sh`: Convert XML camera parameters file to VSRS/DERS camera array format
* `pointcloud/`: Generate XYZRGB `.ply` point cloud from VSRS config file and `.yuv` texture/disparity data
* `extrinsic_convert/`: Operations on VSRS/DERS camera array format
* `kinect_tools//fetch_intrinsics`: Fetch internal camera parameters from Kinect connected by USB, store in JSON file
* `kinect_tools//depth_reprojection`: Reproject and upsample Kinect raw depth map into same coordinate system as color image
* `kinect_tools//vsrs_disparity`: Convert reprojected Kinect depth map into disparity map for VSRS in `.yuv` format
* `kinect_tools/depth2disparity.sh`: From raw Kinect depth map to VSRS disparity map
* `kinect_tools/batch_depth2disparity.sh`: Apply `depth2disparity.sh` for all files in a directory.
