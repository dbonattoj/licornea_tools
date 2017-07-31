# misc/view\_syn

GUI to test view synthesis on dataset in real time, without VSRS.

    misc/view_syn dataset_parameters.json cameras.json [dataset_group]
    
Loads [dataset](data/dataset.html) [dataset_parameters.json] and [camera array](data/cameras.json). Optionally use images and depth from the dataset group `dataset_group`, instead of the main group.

It does a simple pixel-wise forward warping with, with depth test.

The sliders in the GUI are used to select a _reference view_ index ("ref X", "ref Y"), and a _target view_ index ("tg X", "tg Y").

The reference view image is put as background. Pixels from the _target view_ are warped onto the _reference view_, and blended onto the image. The opacity can be set with the "opacity" slider. When it is such that both images are visible (i.e. not at either extremum), it becomes visible if pixels get warped to the correct place, with the given camera parameters.

The "darken" slider additionally darkens the background pixels, onto which no target view pixels could be warped.

