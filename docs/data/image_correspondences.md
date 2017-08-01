# Image correspondences

An _image correspondence feature_ contains the position of one same _feature_ across a set of _views_. The _views_ are images of a [dataset](dataset.html). The _feature_ is one common object point, seen images captured from different positions. Optionally, it can also store a depth, and a weight for the feature point on each view.

_Image correspondences_ are a set of _image correspondence features_. The features may cover different subsets of views, and may have different reference views.

The _image correspondence feature_ can optionally have one _reference view_. It means that for the reference view, the feature position was freely chosen, whereas for the other views, it was chosen so as to correspond to the same object position. The _Image correspondences_ can contain features with different reference views.

A [feature points](feature_points.html) file stores the set of feature points (position, depth, weight), for only one view.

The tool [calibration/cors\_info](../tools/calibration/cors_info.html) displays information about an image correspondences file.

To merge multiple _image correspondences_ files together into one, use [calibration/merge\_cors](../tools/calibration/merge_cors.html). This does not lose any information.

### Optical flow

The tool [calibration/cg\_optical\_flow\_cors](../tools/calibration/cg_optical_flow_cors.html) generates image correspondences by tracking the optical flow of features on all images of a dataset. The features to track on the reference view are chosen using the tool [calibration/cg\_optical\_flow\_features](../tools/calibration/cg_optical_flow_features.html).

### Visualization

The tools [calibration/cg\_cors\_viewer\_f](../tools/calibration/cg_cors_viewer_f.html) and [calibration/cg\_cors\_viewer\_v](../tools/calibration/cg_cors_viewer_v.html) are used to visualize image correspondences in a GUI. 

[calibration/cg\_cors\_viewer\_f](../tools/calibration/cg_cors_viewer_f.html) shows the position of a given feature on the different views.

[calibration/cg\_cors\_viewer\_v](../tools/calibration/cg_cors_viewer_v.html) shows the positions of the features on a given view.

### File format

_Image correspondences_ can be stored either in a JSON format, or in a binary format. For large number of image correspondences (such as optical flow on a dense dataset), the binary format takes up much less space, and loads faster.

On **every** tool that takes a `.json` argument for a image correspondences file (input or output), giving a `.bin` filename instead uses the binary format.

The tool [calibration/copy\_cors](../tools/calibration/copy_cors.html) copies image correspondences from one file to another, enabling conversion from/bin binary.
