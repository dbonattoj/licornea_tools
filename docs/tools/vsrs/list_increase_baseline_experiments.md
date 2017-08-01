# vsrs/list\_increase\_baseline\_experiments

Make 1D experiments list where baseline is increased.

    vsrs/list_increase_baseline_experiments parameters.json center max_baseline out_experiments.json [step]

Virtual view index is fixed to `center`. Baseline increases from 0 to `max_baseline`, in steps of `step` (default 1, or whatever `x_step` dataset parameters have).

Dataset must be 1D. Use [dataset/slice](../dataset/slice.html) to take a 1D slice of a 2D dataset.
