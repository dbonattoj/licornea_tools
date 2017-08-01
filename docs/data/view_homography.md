# View homography

A _view homography_ file contains a 3x3 homography matrix to apply to an image. A _view homographies_ file contains a _view homography_ for each view index in a [dataset](dataset.html).

The homography matrix is a projective transformation in homogeneous coordinates, which maps pixel positions of the image to a new position.

It can be applied to one image using [misc/apply\_homography](../tools/misc/apply_homography.html). A border (positive or negative) such that the resulting images do not get cropped, can be found using [misc/homography\_maximal\_border](../tools/misc/homography_maximal_border.html). This works either for one _view homography_, or one border for the whole set of _view homographies_.
