# calibration/cg\_redistribute\_cors

Subdivide [image correspondences](../../data/image_correspondences.html) into overlapping subsets with different reference views.

    calibration/cg_redistribute_cors in_cors.json pseudo_refgrid.json outreach_radius out_cors.json

For example: The input `in_cors.json` has 2 features `A` and `B`, on 1 reference view `r`, both tracked over an outreach of 10:

    A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A  A
    B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B  B
                                  r
                                                    -----> v

For every view `v`, `A` and `B` should correspond to the same scene object point. There can be an error in this because of the optical flow estimation. But for the reference view `r` there is no error (because the features were defined on that view). 

The new `pseudo_refgrid.json` is given with two additional _pseudo-reference views_ left and right of `r`. Then this redistribution is done with an outreach of 5. The resulting `out_cors.json` now have 6 features `A1, A2, A3, B1, B2, B3` on 3 different reference views `r1, r2, r3`:


    A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 .  .  .  .  .  .  .  .  .  .  .
    B1 B1 B1 B1 B1 B1 B1 B1 B1 B1 .  .  .  .  .  .  .  .  .  .  .
                r1                r2                r3
    
    
    .  .  .  .  .  A2 A2 A2 A2 A2 A2 A2 A2 A2 A2 A2 .  .  .  .  .
    .  .  .  .  .  B2 B2 B2 B2 B2 B2 B2 B2 B2 B2 B2 .  .  .  .  .
                r1                r2                r3
    
    
    .  .  .  .  .  .  .  .  .  .  .  A3 A3 A3 A3 A3 A3 A3 A3 A3 A3
    .  .  .  .  .  .  .  .  .  .  .  B3 B3 B3 B3 B3 B3 B3 B3 B3 B3
                r1                r2                r3
    
                                                    -----> v


Here, for the reference views `r1` and `r3`, there can now be an error in `A` and `B` (they no longer are on the right scene object point). So they are _pseudo-reference views_.

The new feature points `A1, A2, A3, B1, B2, B3` are copies of `A, B, C`. So the `out_cors.json` gets much larger. (Binary format `out_cors.bin` should be used).

When used with [calibration/cg\_rcpos\_from\_cors](cg_rcpos_from_cors.html), it will estimate [relative camera positions](../../data/relative_camera_positions.html) for each of the new _pseudo-reference views_ as center, to the error does not get as large since the outreach is smaller. Instead more error will come from stiching in [calibration/cg\_stitch\_cameras](cg_stitch_cameras.html).
