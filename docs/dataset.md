# Dataset parameters

A _dataset_ is a collection of images, depth maps, camera parameters, etc. from an acquisition.
The _dataset parameters_ file defines how the image, depth map, etc. files are organized and numbered on the file system.

All of these tools use a common JSON-based dataset parameters file format. It can describe both 1D and 2D datasets.
There can be different versions of equivalent files (e.g. for each view, Kinect raw depth map, upsampled depth map, and
VSRS disparity map). There can be a different numbering for these files.

The classes implemented in `src/lib/dataset.h` (C++) and `src/lib/pylib/dataset.py` (Python) are used by all other tools to
read the dataset parameters.

## Example

### 2D dataset parameters file:

```
{
	"y_index_range" : [101, 201],
	"x_index_range" : [1, 851],	
	
	"camera_name_format" : "cam_{y:04d}{x:04d}",
	"cameras_filename" : "cameras.json",

	"width" : 1920,
	"height" : 1080,

	"image_filename_format" : "files/image_{y:04d}{x:04d}.png",
	"depth_filename_format" : "files/depth_{y:04d}{x:04d}.png",

	"vsrs" : {
		"z_far" : 1600.0,
		"z_near" : 600.0,
		"image_filename_format" : "../3DLicornea_A1/textures/Band2_0101-0201_step1mm/files/cam_{y:04d}{x:04d}.yuv",
		"depth_filename_format" : "../3DLicornea_A1/disparity/Band2_0101-0201_step1mm/files/cam_{y:04d}{x:04d}.yuv"
	},
	
	"kinect_raw" : {
		"filename_y_index_factor" : -1,
		"filename_y_index_offset" : 684,
		"filename_x_index_factor" : 1,
		"filename_x_index_offset" : 0,
		"image_filename_format" : "../raw/483-583_step1mm_acq2/texture/{y}.0/Kinect_out_texture_000_{y}.0z_0001_{x:04d}.png",
		"depth_filename_format" : "../raw/483-583_step1mm_acq2/depth/{y}.0/Kinect_out_depth_000_{y}.0z_0001_{x:04d}.png"
	}
}
```


### 1D dataset parameters file:

```
{
    "x_index_range": [1, 851],

    "camera_name_format": "cam_0151{x:04d}", 
    "cameras_filename": "cameras.json", 

    "width": 1920, 
    "height": 1080, 

    "image_filename_format": "files/image_0151{x:04d}.png", 
    "depth_filename_format": "files/depth_0151{x:04d}.png", 

    "vsrs": {
        "z_far": 1600.0, 
        "z_near": 600.0,
        "image_filename_format": "../3DLicornea_A1/textures/Band2_0101-0201_step1mm/files/cam_0151{x:04d}.yuv", 
        "depth_filename_format": "../3DLicornea_A1/disparity/Band2_0101-0201_step1mm/files/cam_0151{x:04d}.yuv" 
    }, 

    "kinect_raw": {
        "filename_x_index_factor": 1, 
        "filename_x_index_offset": 0, 
        "image_filename_format": "../raw/483-583_step1mm_acq2/texture/533.0/Kinect_out_texture_000_533.0z_0001_{x:04d}.png",
        "depth_filename_format": "../raw/483-583_step1mm_acq2/depth/533.0/Kinect_out_depth_000_533.0z_0001_{x:04d}.png"
    }
}

```

## Format
The parameters file is a JSON file. The index ranges (`x_index_range`, `y_index_range`), image+depth
filenames (`image_filename_format`, `depth_filename_format`), camera parameters, other attributes are global to the
dataset. The object `vsrs` and `kinect_raw` are _file groups_. They contain different versions of the files, possible with different
numbering.

Any extra attributes added to the files are ignored by the tools. Tools that modify parameter files (in `dataset/`) leave the
extra attributes in place.

### Indices
This is a 2D dataset, having X and Y indices. The possible index ranges are given by `x_index_range` and `y_index_range`.
**The minimal and maximal index values are inclusive. (i.e. x=851 is a valid index.)** It is different from Python `range()`'s.

The index is just for numbering, and has not direct relation to the camera arrangement. Each index `(x,y)` corresponds to a _view_.
The view has a camera, image file and depth file. `camera_name_format` is a format template that gives the name of the camera
corresponding to view `(x,y)`. (more below)

2D parameters must contain both `x_index_range` and `y_index_range`. 1D parameters must contains only `x_index_range`.
**It cannot contain only `y_index_range`**. (Because otherwise all the tools would need to distinguish between X-only and Y-only datasets.)

The indices can also have a _step_: specifying for example `[1, 10, 4]` (array with 3 integers) for an index range corresponds to the index values
`1, 5, 9`. `2` would be an _invalid_ index. The maximum `10` is no longer included in this example.


### Format templates
Each view corresponds to a camera name, and an image and depth filename. The format strings are in Python syntax. For
2D datasets, there is an argument "x" and an argument "y". For 1D datasets there is only an argument "x".
Format is described at [https://pyformat.info/](https://pyformat.info/).
The C++ tools use [fmt](https://github.com/fmtlib/fmt) to parse the format string (not `printf`).


### Filenames
"Filenames" are relative file paths. The file paths are always relative to the location of the parameters file.
(All tools read them that way.)


### Groups
`kinect_raw` and `vsrs` are _groups_. They contain additional files belonging to the view. `kinect_raw` are the raw,
unprocessed files from the Kinect camera. `vsrs` are `.yuv` files for VSRS.

They can have a different numbering. `filename_x_index_factor` ("_factor_") and `filename_x_index_offset` ("_offset_") are both optional. If they
are not present, _factor_ defaults to 1, and _offset_ defaults to 0. (Same for Y).
The _factor_ can be a real number. The _offset_ is always an integer. Both can be negative.

The filename format templates in a group (e.g. `vsrs/image_filename_format`) are formatted with the _local_ indices.
The mapping is:

> `local_x = floor(x * factor) + offset`

In this example `x = 101, 102, 103` becomes in `raw_kinect`: `local_x = 684, 683, 682`.


## Manipulation
There are tools for manipulating dataset parameter files in `dataset/`.

`dataset/slice.py` make a 1D slice of a 2D parameters file, by fixing the Y index to a certain value.
It produces a 1D dataset parameters file. The example 1D parameters file was generated using this, by fixing Y to 151.

To instead make a slice fixing the X index: It is not possible to have only a Y index in the file.
Instead `dataset/flip.py` can be used to _swap_ the meanings of the Y and X index. Then `dataset/slice.py` is used 
on that new, flipped parameters file.
