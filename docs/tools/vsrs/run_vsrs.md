# vsrs/run\_vsrs

Run VSRS to synthesize a view on a given dataset. Automatically converts files and generates VSRS configuration and camera files, as required.
      
    vsrs/run_vsrs.py vsrs_binary parameters.json left_idx virtual_idx right_idx output_virtual.png [cameras.txt]

Uses VSRS to do view synthesis of view `virtual_idx` from views `left_idx` and `right_idx`.

`vsrs_binary` is the VSRS executable to use.

The [dataset parameters](../../data/dataset.html) must define the texture and depth (i.e. disparity) YUV files for the `vsrs` group. But it is not necessary for all the YUV files to exist already: 
This program automatically generates the YUV textures and/or disparity maps for left and/or right views, if they don't exist yet, the same way as [vsrs/export\_to\_vsrs](export_to_vsrs.html).

2D view indices can be given in format `x,y` (without space).

`cameras.txt` is camera parameters file in VSRS format, and can be generated before using [camera/export\_mpeg](../camera/export_mpeg.html). If it is not given, a temporary file for it is generated automatically.

The virtual view from VSRS is automatically converted to PNG format for easy usage.

All of the intermediary temporary files that may be generated (cameras, configuration, YUV output), get unique names at each execution, and are removed in the end. The script can safely be executed multiple times in parallel.
