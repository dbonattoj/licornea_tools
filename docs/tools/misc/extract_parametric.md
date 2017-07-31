# misc/extract\_parametric

Copy sequence of dataset images where indices move on a parametric curve.

    misc/extract_parametric parameters.json out_dirname/ frames

Copies `frames` images from the dataset into files in `out_dirname/` with sequentially numbered names, where the view indices evolve according to a parametric equation `(x,y) = f(t)`.

The equation can be set in the script (function `f(t)`). `t` evolves from 0 to 1 (in `count`) steps, and it must return two values `x, y` from 0 to 1. The program converts this into valid view indices.

Preset are a figure 8, a spiral, a circle.
