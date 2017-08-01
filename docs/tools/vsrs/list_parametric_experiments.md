# vsrs/list\_parametric\_experiments

Make [experiments](../../data/experiments.html) list where virtual view moves on a parametric curve.

    vsrs/list_parametric_experiments parameters.json x_key y_key frames out_experiments.json

Dataset must be 2D. Curve is defined inside script, as with [misc/extract\_parametric](../misc/extract_parametric.html).

Input views are on a regular index grid, with `x_key` and `y_key` intervals.

For each virtual view, the closest leftside and rightside input views are used. (not perfect)
