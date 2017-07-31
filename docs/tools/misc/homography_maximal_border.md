# misc/homography\_maximal\_border

Compute border needed to apply one or several homographies without clipping.

    misc/homography_maximal_border homography.json/homographies.json width height out_border.json

The first argument can either be one [view homography](data/view_homography.html), or multiple view homographies. (One for each view in a dataset, for example for rectification.) `width` and `height` are the size of the images on which these homographies will be applied.

It writes the border in `out_border.json`, such that applying any of these homographies on an image of the given size, will never result in clipping.
