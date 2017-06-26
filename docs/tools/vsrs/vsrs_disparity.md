# vsrs/vsrs\_disparity

Convert depth map to YUV disparity map for use with VSRS.

    vsrs/vsrs_disparity depth.png out_disparity.yuv z_near z_far [8/16]
    
## Usage
  
`depth.png` is a 16 bit depth image, where pixel values are orthogonal distances. `z_near` and `z_far` must be set to the minimal/maximal depth values in the region of interest. A YUV file is written to `out_disparity.yuv`. It is by default 8 bit, but can be set to 16 bit if `16` is put as the last argument.

## Description

The output YUV file is a 3 channel YUV240 file, but only the first channel (Luma Y) is filled with depth values. It is padded to twice that size to add empty UV channels, because this is needed by VSRS.

The values for `z_near` and `z_far` need to be given to VSRS as `LeftNearestDepthValue` and `LeftFarthestDepthValue` (same for `Right`) in its config file. To use 16 bit disparity maps, VSRS needs to be recompiled with the appropriate flag.

Only depth values `z` inside `[z_near, z_far]` will be mapped to disparity values `d`. Disparity values are discrete integer values. The mapping is non-linear, for depth closer to `z_near`, the disparity graduation is finer. The smaller `z_far - z_near`, the higher the detail in the disparity map.

The mapping is `d = a + b/z`, with `a = -255 z_near / (z_far - z_near)` and `b = 255 z_near z_far / (z_far - z_near)`. 255 becomes 65535 for 16 bit disparity maps. `d` is a _projected depth_ values VSRS uses with its projection matrices to do 3D warping.