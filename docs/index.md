# Documentation

These are several programs that do single processing steps in the 3DLicorneA pipeline. Each program works independently, taking some input data and generating output data. Parameters are usually exchanged in JSON-based formats.

The programs are organized into thematic groups, in the different subdirectories `vsrs/`, `kinect/`, etc. Each `.py` and each `.cc` file is one program. Only the C++ programs operate on the content of images. Some Python programs are scripts that call other programs on a set of inputs, to do batch operations. (For example, exporting an entire dataset to VSRS format).

Building installs all the executables/scripts into the `bin/` directory, from which they should be used.

- [Installation and usage](installation.html)
- [Dataset parameters](dataset.html)
- [Camera parameters](camera.html)


## Usage examples
Examples on how to use the tools for common tasks.

- Import dataset from Kinect
- Calibrate camera parameters
- Export dataset to VSRS
- Run view synthesis experiments using VSRS

## Tools

### Calibration
Calibrate intrinsic camera parameters using object-to-image correspondences. Rectify and calibrate dataset using optical flow.

---

### Camera
Manipulate camera parameters. Transform to/from different formats. Visualize cameras.
Operate on [camera.json](camera.html) camera parameters files.

- [**export mpeg**](tools/camera/export_mpeg.html): Export cameras to MPEG (VSRS/DERS) format.
- [**transform**](tools/camera/transform.html): Transform parameters and/or namings of camera parameters.
- [**visualize**](tools/camera/visualize.html): Generate 3D visualization of camera parameters.

---

### Kinect
Connect to Kinect v2. Calibrate intrinsics, and IR-to-color reprojection using checkerboards. Reproject and upsample depth maps.

- [**depth reprojection**](tools/kinect/depth_reprojection.html): Reproject and upsample raw Kinect depth map onto color image.
- [**import raw data**](tools/kinect/import_raw_data.html): Batch import Kinect acquisitions from entire dataset.

---

#### Live Kinect tools

These programs interact with a Kinect v2 connected via USB. They are only available if the tools were built with libfreenect2.

- [**viewer**](tools/kinect/viewer.html): View color, IR and depth feeds from Kinect v2.
- [**fetch internal parameters**](tools/kinect/fetch_internal_parameters): Fetch internal parameters stored inside the Kinect, and store in JSON file.
- [**internal ir intrinsics**](tools/kinect/internal_ir_intrinsics.html): Take IR intrinsics from internal parameters.
- [**checkerboard samples**](tools/kinect/checkerboard_samples.html): Take pictures with a checkerboard from color and/or IR, and same the corner pixel coordinates.
- [**color intrinsic reprojection**](tools/kinect/color_intrinsic_reprojection.html): Using calibrated camera matrix, visually reproject detected checkerboard corners, to measure quality of camera matrix.
- [**ir intrinsic reprojection**](tools/kinect/ir_intrinsic_reprojection.html): Same for IR/depth camera.
- [**ir distortion viewer**](tools/kinect/ir_distortion_viewer.html): Visualize distortion parameters in IR intrinsics.
- [**remapping viewer**](tools/kinect/remapping_viewer.html): Compare calibrated reprojection parameters, to remapping using Kinect's proprietary method.
- [**reprojection viewer**](tools/kinect/reprojection_viewer.html): Visualize reprojection of depth image to color image, and compare depth values.
- [**checkerboard depth viewer**](tools/kinect/checkerboard_depth_viewer.html): Compare calculated depth of checkerboard in IR image, with measurements of Kinect depth sensor.
- [**checkerboard depth samples**](tools/kinect/checkerboard_depth_samples.html): Take large number checkerboard depth samples, independent of camera matrix.
- [**checkerboard depth stat**](tools/kinect/checkerboard_depth_stat.html): Using a camera matrix, convert these samples into text file that can be analyzed with MATLAB.
- [**checkboard color depth**](tools/kinect/checkboard_color_depth.html): Same, but with color image, reprojection parameters, and reprojected depth image.
- [**fetch intrinsics**](tools/kinect/fetch_intrinsics.html): Fetch internal intrinsic parameters stored inside the Kinect.


### VSRS
Export datasets to VSRS, batch run VSRS.

- [**vsrs disparity**](tools/vsrs/vsrs_disparity.html): Convert depth map to VSRS disparity map in YUV format.
- [**export for vsrs**](tools/vsrs/export_for_vsrs.html): Batch export entire dataset (images and depth maps) to VSRS formats.
- [**make vsrs config**](tools/vsrs/make_vsrs_config.html): Generate VSRS configuration file for given data set and given views.
- [**run vsrs**](tools/vsrs/run_vsrs.html): Run VSRS for given data set and given views, automatically generating configuration file.
- [**run vsrs experiments**](tools/vsrs/run_vsrs_experiments.html): Batch run VSRS a given set of experiments.

---

### Dataset
Manipulate dataset parameters files.

- [**flip**](tools/dataset/flip.html): Flip X and Y coordinates on 2D dataset.
- [**slice**](tools/dataset/slice.html): Generate 1D dataset from 2D dataset by fixing Y coordinate.

---

### Miscellaneous
Other tools.

- [**view_depth**](tools/misc/view_depth.html): GUI to visualize depth maps.
- [**psnr**](tools/misc/psnr.html): Calculate PSNR between two images.

