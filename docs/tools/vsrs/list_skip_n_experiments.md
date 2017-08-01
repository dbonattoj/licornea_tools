# vsrs/list\_skip\_n\_experiments

Make 1D experiments list where every _n_-th view is taken as input.

    vsrs/list_skip_n_experiments parameters.json step out_experiments.json [limit]

Every `step`'th view is taken. Virtual view sweeps from left to right. Optionally limited by `x < limit`.

Dataset must be 1D. Use [dataset/slice](../dataset/slice.html) to take a 1D slice of a 2D dataset.
