# VSRS Experiments

This is a list of view synthesis executions to be done by VSRS. Each _experiment_ is a 3-tuple of (1D or 2D) [view indices](dataset.html): For the left view, right view, and the virtual view to synthesize.

Such experiment list can be generated using [vsrs/list\_increase\_baseline\_experiments](../tools/vsrs/list_increase_baseline_experiments.html), [vsrs/list\_parametric\_experiments](../tools/vsrs/list_parametric_experiments.html), and [vsrs/list\_skip\_n\_experiments](../tools/vsrs/list_skip_n_experiments.html).

They can be executed (parallelized) using [vsrs/run\_vsrs\_experiments](../tools/vsrs/run_vsrs_experiments.html) and with [vsrs/psnr\_experiments](../tools/vsrs/psnr_experiments.html). The first stores the output images in a sequence. The second computes the PSNR comparing each virtual view to the corresponding real view.
