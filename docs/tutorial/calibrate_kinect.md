# Calibrate Kinect intrinsics and reprojection

The tools in `kinect/` can be used to calibrate the intrinsic and reprojection parameters of a **Kinect v2**. Its depth map is take using an IR (infra-red) camera, that has a lower resolution, and a different camera pose. Reprojection is the mapping of pixels from the depth/IR image onto pixels of the color camera.

The method provided by Freenect2 (and the Microsoft SDK) only maps pixel positions. It is not meant to use the remapped depth image, as depth map of the color image: Depth values are slightly incorrect because they are unchanged, and still represent distances from the IR sensor. And they have a slight absolute offset.

This calibration takes this into account. It estimates an intrinsic matrix with distortion for the color and the IR camera, a rigid transformation matrix between them, and optionally a depth offset model on the input and/or output depth map. It is done using checkerboard calibration patterns.

## 1. Take checkerboard samples

### Color camera

After the tools are [installed](../installation.html) (with **libfreenect2**) and the Kinect is connected, run

    kinect/checkerboard_samples color cols rows square_width color_cors.json

`cols` and `rows` are the number of corners (horizontal and vertical) of the checkerboard. The outer corners don't count. `square_width` is the side length of each square, in mm. The checkerboard should be as flat as possible.

The program shows visually the detected corners on the video stream of the _color_ camera. Hit _Enter_ to record one sample. Take many (about 100) samples of the checkerboard, placed at different places in the image, and with different orientations. The orientations should vary by amounts of about 10°. There should be samples where the checkerboard is as near as possible to the edges of the image. When done, hit _Esc_ to exit, and save the samples `color_cors.json` as [1-object-1-image correspondences set](../data/obj_img_cors.html).


### IR camera

Next, run

    kinect/checkerboard_samples ir cols rows square_width ir_cors.json

And take samples of the checkerboard in the IR image the same way. It can be useful to cover the Kinect's IR light with a translucent piece of paper, etc. when the checkerboard is nearby. It can make the checkerboard visible again, if it is too reflective.

There should be many samples where the checkerboard is near the edges, because this area is most affected by distortions.

### Both cameras

Then, run

    kinect/checkerboard_samples both cols rows square_width both_cors.json
    
And take samples where the checkerboard is visible in with both cameras at the same time. It is important to wait until the checkerboard is no longer moving before taking a sample: The color and IR streams are out of sync, so if it is moving, the corners will be at different positions in the two images.

The output `both_cors.json` will be a [1-object-2-image correspondences set](../data/obj_img_cors.html).



## 2. Calibrate intrinsics

### Merge correspondences (optional)
If there are not enough samples in `color_cors.json` or in `ir_cors.json`, then the ones from `both_cors.json` can be added to it using

    misc/cat_obj_img_cors color_cors.json both_cors.json color_cors.json 1

respectively

    misc/cat_obj_img_cors ir_cors.json both_cors.json ir_cors.json 2

### Color intrinsics

To calibrate the intrinsic parameters of the color camera, use

    calibration/calibrate_intrinsics color_cors.json 1920 1080 color_intr.json no_distortion
    
The color image has no (or very little) distortion, so it is better to use `no_distortion`.

To verify the calibration, connect the Kinect and use

    kinect/color_intrinsic_reprojection cols rows square_width color_intr.json
    
It will detect corners on a checkerboard, and measure the reprojection error.


### IR intrinsics

To calibrate the intrinsic parameters of the IR camera, use

    calibration/calibrate_intrinsics ir_cors.json 512 424 ir_intr.json
    
Here distortion must also be calibrated. To verify it, use

    kinect/ir_intrinsic_reprojection cols rows square_width ir_intr.json



## 3. Calibrate rigid transformation

To calibrate the rigid transformation between the color and IR spaces, use

    kinect/calibrate_color_ir_reprojection both_cors.json color_intr.json ir_intr.json reprojection.json


It will compute the [reprojection parameters](../data/reprojection.html).

### Evaluate remapping

To compare these reprojection parameters to the remapping with the method from the Microsoft Kinect, first extract the internal parameters of the Kinect. When the Kinect is connected, use

    kinect/fetch_internal_parameters internal.json

Then run

    kinect/remapping_viewer internal.json reprojection.json
    
It will remap the IR image to color image using both methods, and compare the result. The error should be very small. This just remaps pixel positions, and not depth values.

### Evaluate depth reprojection

To evaluate the reprojection of the depth, use

    kinect/reprojection_viewer reprojection.json

It reprojects (and upsamples) the depth map in real time, and shows the difference in the depth values.


## 4. Calibrate depth offset (optional)

The reprojection parameters can also include a polynomial depth offset. It is a value to be added to either the input depth (in IR space), and/or the output reprojected depth (in color space). It is a polynomial function of the `(x,y)` pixel position.

### Input depth offset 

With a checkerboard and the Kinect connected, run

   kinect/checkerboard_depth_viewer cols rows square_width ir_intr.json

It detects the checkerboard on the IR image, and compares measured depth in it (from the depth map), to calculated values (from the checkerboard geometry). They should be the same. It not it is (partially) because of the absolute offset in the Kinect's depth.

To take samples of the depth offset, use

   kinect/checkerboard_depth_samples cols rows chk_samples.json

Take many samples, with the checkerboard at different positions and depths. It should always be kept more or less parallel to the Kinect.

Then run

    kinect/checkerboard_depth_stat chk_samples.json cols rows square_width ir_intr.json chk_stat.txt

The output `chk_stat.txt` contains many samples comparing depths for pixels in the checkerboard. Use **MATLAB** or another program to compute a model of the offset, in function of the pixel position `(x,y)`. It can be linear or polynomial to the second degree.

Then add these parameters to `reprojection.json`, under `ir_intrinsics`, like:

```
{
    "..." : "...",
    "ir_depth_offset" : {
        "x0y0" : -2.50478486530832711,
        "x1y0" : -0.00639290343549093,
        "x0y1" : 0.07508821095407339,
        "x2y0" : 0.0,
        "x0y2" : 0.0,
        "x1y1" : 0.0
    }
}
```

The field **x2y0** represents the coefficient of x^2, for example. The function

> offset(x,y) = **x0y0** + **x1y0** x + **x0y1** y + **x2y0** x^2 + **x0y2** y^2 + **x1y1** x y
    
Should correspond to _raw - corrected_, where _raw_ is the input depth value, the _corrected_ the new depth value. (which should equal the calculated depth from the checkerboard geometry in the samples.)


### Output depth offset

A depth offset model can also be added to the output reprojected depth. To collect samples of the depth offset on color image checkerboard v. reprojected depth maps, use:

    kinect/checkerboard_color_depth cols rows square_width reprojection.json chk_stat.txt

Then use **MATLAB** similarly on the samples `out_stat.txt`. The depth offset mode should be put into `color_depth_offset` in `reprojection.json`.
