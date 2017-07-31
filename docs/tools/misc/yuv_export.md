# misc/yuv\_export

Convert color or monochrome PNG image to raw format.

    misc/yuv_export image.png out_image.yuv ycbcr420/rgb_planar/rgb_interleaved/mono8/mono16
    
Formats:

- `ycbcr420`: planar YUV with YCbCr color space, 4:2:0 chroma subsampling
- `rgb_planar`: planar RGB
- `rgb_interleaved`: interleaved RGB
- `mono8`: 8 bit monochrome
- `mono16`: 16 bit monochrome

To generate VSRS disparity maps from depth maps, [vsrs/vsrs\_disparity](tools/vsrs/vsrs_disparity.html) must instead be used.

