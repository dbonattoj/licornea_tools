# dataset/duplicates

Finds duplicated adjacent views in the dataset.

    dataset/duplicates dataset_parameters.json +x/-x/+y/-y [bad_files.txt] [dataset_group]

When images a taken sequentially, an image is a _duplicate_ when it is the same as the previously taken picture. The second argument must be set to how the indices increase between sequential images. For example `-x` means that for every next image, the `x` index decrements.

Shows bad (duplicate) image and depth files in output, optionally writes them to `bad_files.txt`.
