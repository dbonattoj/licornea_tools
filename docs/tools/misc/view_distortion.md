# misc/view\_distortion

GUI to visualize effect of intrinsic camera distortion parameters.

    misc/view_distortion intr.json
    
`intr.json` are [intrinsic parameters](../../data/intrinsics.json) of a calibrated camera.

Displays sliders to adjust values of the distortion coefficients `k1`, `k2`, `k3`, `p1` and `p2`. Displays four images:

In the upper row, the left image is a regular lattice of points. Its density can be ajdusted with the "cell width" slider. The right image shows the positions of the same points on an image after distortion is applied.

In the lower row, the right image is a regular lattice of points. The left image shows the positions of the same points on an image after distortion is removed.
