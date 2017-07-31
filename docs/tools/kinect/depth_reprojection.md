# kinect/depth\_reprojection

Reproject raw Kinect depth map to color image.

    kinect/depth_reprojection input.png output.png/- output_mask.png/- reprojection_parameters.json method [was_flipped]

Reprojects and upscales raw Kinect depth map, to a depth map of the color image. In the resulting reprojected depth map, values are distances from the color camera's optical center, orthogonal to the color camera plane. (In the raw depth map, it is the IR camera, and there is distortion.)

`input.png` must be a 16-bit monochrome PNG image. `output.png` will have the same format.

`output.png` and `output_mask.png` can be set to `-` if they should not be used.

`reprojection_parameters.json` are the (reprojection parameters)[data/reprojection.html] obtained from [kinect/calibrate\_color\_ir\_reprojection](tools/kinect/calibrate_color_ir_reprojection.html).

`method` is the upsampling/densification algorithm to use. Choices are:

- `fast`: Simple nearest neighbor interpolation, very fast.

- `splat`: Simple splatting algorithm.

- `mine`: The one used for the Licornea datasets, described in the MPEG document of the first dataset.

`was_flipped` is set by default. (Give another argument value to disable it). It indicates that the input depth map taken from the Kinect was already flipped on the vertical axis. Then is must flip it back, reproject it, and again flip the result; reprojection is not symmetric.
