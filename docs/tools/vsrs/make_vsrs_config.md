# vsrs/make\_vsrs\_config

Make VSRS configuration file from predefined template.
  
    vsrs/make_vsrs_config.py dataset_parameters.json cameras.txt left_idx virtual_idx right_idx output_virtual.yuv output_config.txt

`dataset_parameters.json` is a [dataset parameters](data/dataset.html) file for a dataset. `cameras.txt` are camera parameters in MPEG format. 

Generates VSRS configuration file that makes VSRS do view synthesis of view `virtual_idx` from views `left_idx` and `right_idx`. The YUV texture and disparity maps for VSRS, must already exist, and be specified in the `vsrs` group of the [dataset parameters](data/dataset.html).

`cameras.txt` is camera parameters file in VSRS format, and must be generated before using [camera/export\_mpeg](tools/camera/export_mpeg.html).

Template for the VSRS configuration file is inside the script. 2D view indices can be given in format `x,y` (without space).
