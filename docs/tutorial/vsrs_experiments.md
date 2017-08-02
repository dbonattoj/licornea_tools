# Run VSRS experiments

It is not necessary to [export the whole dataset to VSRS](export_to_vsrs.html) to run VSRS on some views. But the [dataset parameters](../data/dataset.html) must have the `vsrs` group set up. The program [vsrs/run\_vsrs](../tools/vsrs/run_vsrs.html) automatically generates the YUV files and VSRS camera parameters on the fly. The `cameras_filename` must also be set for this.

## Run single view synthesis

To synthesize a single view, run

    vsrs/run_vsrs path/to/vsrs/ViewSyn parameters.html 200,100 300,100 400,100 virtual.png

If will synthesize virtual view at [index](../data/dataset.html) `(x=300, y=100)`, from left view `(x=200, y=100)` and right view `(x=400, y=100)`. For a 1D dataset, use numbers like `300` instead of `300,100`.

Internally the program generates temporary files (VSRS camera parameters and configuration, YUV output) with unique names per invokation; it is safe to run is multiple times in parallel.

## Run VSRS multiple experiments

Prepare a sequence of [VSRS experiments](../data/experiments.html) using [vsrs/list\_increase\_baseline\_experiments](../tools/vsrs/list_increase_baseline_experiments.html), [vsrs/list\_parametric\_experiments](../tools/vsrs/list_parametric_experiments.html), or [vsrs/list\_skip\_n\_experiments](../tools/vsrs/list_skip_n_experiments.html).

To run VSRS for each one and save the sequence of output images, run

    vsrs/run_vsrs_experiments path/to/vsrs/ViewSyn parameters.html experiments.json out_virtual/

It will store output images (sequentially numbered) in the directory `out_virtual/`.

To instead evaluate the PSNRs, run

    vsrs/psnr_experiments path/to/vsrs/ViewSyn parameters.html experiments.json results.txt
