# dataset/flip

Flip dimensions of 2D dataset.

    dataset/flip.py in_parameters.json out_parameters.json

Swaps all references to `x` index to `y`, and vice versa. Traverses all dataset groups.

To create a vertical (`y` changes) slice of a dataset, do this first, then use [dataset/slice](slice.html) on the flipped dataset parameters.
