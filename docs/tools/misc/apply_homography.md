# misc/apply\_homography

Distorts an image given a [view homography](data/view_homography.html).

    misc/apply_homography homography.json in_image.png out_image.png texture/depth [border.json]
    
Reads the image `in_image.png`, applies the homography matrix from `homography.json`, and writes the output image into `out_image.png`.

The third parameter indicates whether `in_image.png` is a `texture` or a `depth` image. If it is a texture, bicubic interpolation is used. Otherwise, nearest neighbor interpolation is used.

If `border.json` is given, that border is added to the output image, making it larger than the input image. (Or smaller, if borders are negative). It should be set so as to avoid clipping.

The homography matrix refers to the mapping of image coordinates _before_ the border is added. The programming internally modifies it as needed.
