# Object-image correspondences

Corresponding points between 2D image(s) and 3D object(s). Stored in JSON files typically called `obj_img_cors.json`.

## Description

### Object-image correspondence point

An _n-object-m-image correspondence point_ is the coordinates of _one same point_ on _n_ 3D objects, and _m_ 2D images.

For example the position of a scene point in world space `(X, Y, Z)`, and the positions of the pixels showning that point on two images `(ix1, iy1)` and `(ix2, iy2)`, would be a _1-object 2-images correspondence_.

### Object-image correspondences

_n-object-m-image correspondence**s**_ are an array of _n-object-m-image correspondence_ points. For all the points, the relative arrangement of the object(s) and image camera(s) are the same.

For example capturing a chessboard pattern from two cameras generates _1-object-2-image correspondences_ with points for all the checkerboard corners; their pixel positions on both images, the their 3D position on the checkerboard's own coordinate system.


### Object-image correspondences set

Finally, a _n-object-m-image correspondences set_ is an array of the latter. (That is, an array of arrays of correspondence points).

The relative arrangement of object(s) and image camera(s) can be different on the outer array. And they can be different images.

For example when a checkerboard is captured multiple times with two cameras, the output data is a 1-object-2-image correspondences set. The placement of the object (the checkerboard) relative to the cameras is different for each image.

## Usage

[kinect/checkerboard\_samples](tools/kinect/checkerboard_samples.html) generates a 1-object-2-image correspondences set with chessboard corners, from images taken with a Kinect's color and IR camera.

[misc/cat\_obj\_img\_cors](misc/cat_obj_img_cors.html) is used to merge multiple object-image correspondences sets together.
