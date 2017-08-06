# camera/merge\_cameras

Combine two camera arrays

    in_cameras1.json in_cameras2.json out_cameras.json [common_camera]

Output `out_cameras.json` will contain cameras from both inputs. If camera with same name exists in both arrays, the one from `in_cameras1.json` is taken.

Can also be used to put two camera arrays into one global coordinate system: `common_camera` may be set to the name of a camera that occurs in both input arrays. If set, then the cameras from `in_cameras2.json` are first transformed such that the extrinsic of `common_camera` in `in_cameras2.json` becomes the same as that in `in_cameras1.json`.
