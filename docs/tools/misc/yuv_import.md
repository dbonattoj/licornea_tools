# misc/yuv\_import

Convert color or monochrome YUV image to PNG.

    misc/yuv_import image.yuv out_image.png width height ycbcr420/rgb_planar/rgb_interleaved/mono8/mono16
   
With and height must be given. Formats:

- `ycbcr420`: planar YUV with YCbCr color space, 4:2:0 chroma subsampling
- `rgb_planar`: planar RGB
- `rgb_interleaved`: interleaved RGB
- `mono8`: 8 bit monochrome
- `mono16`: 16 bit monochrome


