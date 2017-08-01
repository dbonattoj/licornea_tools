# dataset/slice

Takes 1D slice of 2D dataset.

    dataset/slice.py in_parameters.json y out_parameters.json
    
Fixes all references to `y` index to the given `y` value. Traverses all dataset group.

To create a vertical (`y` changes and `x` is fixed) slice of a dataset, first use [dataset/flip](flip.html) to flip the dataset, then take a slice of the flipped dataset, where `y` is fixed to the value that `x` should have been fixed to.
