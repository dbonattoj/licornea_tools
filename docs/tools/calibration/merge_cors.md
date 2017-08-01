# calibration/merge\_cors

Merge differnet [image correspondences](../../data/image_correspondences.html) files into one file.

    calibration/merge_cors in_cors1.json in_cors2.json out_cors.json

Merges `in_cors1.json` and `in_cors2.json`, into `out_cors.json`. (For each argument `.bin` format can also be used). This does not lose any information. If the two input image correspondence files are from different reference views, then that information is still retained in `out_cors.json`. (The two inputs can also already be with multiple reference views).

To merge more than two image correspondences files, call the program multiple times, like

    calibration/merge_cors cors1.json cors2.json all_cors.json
    calibration/merge_cors all_cors.json cors3.json all_cors.json


