# vsrs/psnr\_experiments

Run VSRS experiments and evaluate each PSNR.

    vsrs/psnr_experiments vsrs_binary parameters.json in_experiments.json out_results.txt

Runs VSRS for each experiments, and compares the resulting virtual view with the corresponding real view from dataset.

PSNR is calculated using [misc/psnr](../misc/psnr.html), and results are written into `out_results.txt`. Runs parallelized, and results are still in correct order.
