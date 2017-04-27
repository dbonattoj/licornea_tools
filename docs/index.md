# Documentation

## Global

- [Installation and usage](installation.html)
- [Dataset parameters](dataset.html)
- [Camera parameters](camera.html)


## Tutorials
Examples on how to use the tools for common tasks.

- Import dataset from Kinect
- Calibrate camera parameters
- Export dataset to VSRS
- Run view synthesis experiments using VSRS

## Tools

### Calibration
Determine intrinsic and extrinsic camera parameters, using checkerboards and/or optical flow.

### Camera
Manipulate camera parameters, transform to/from MPEG camera parameters file, visualize cameras.

### Kinect
Import data from Kinect, reproject and upsample depth maps.

- [**fetch_intrinsics**](tools/kinect/fetch_intrinsics.html): Fetch internal intrinsic parameters stored inside the Kinect.
- [**depth_reprojection**](tools/kinect/depth_reprojection.html): Reproject and upsample raw Kinect depth map onto color image.
- [**import_raw_data**](tools/kinect/import_raw_data.html): Batch import Kinect acquisitions from entire dataset.

### VSRS
Export files to VSRS, batch run VSRS.

### Dataset
Manipulate dataset parameters files.

### Miscellaneous
Other tools.

- [**view_depth**](tools/misc/view_depth.html): GUI to visualize depth maps.
- [**psnr**](tools/misc/psnr.html): Calculate PSNR between two images.


## Implementation details

