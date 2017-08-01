# misc/cat\_obj\_img\_cors

Merge two [object-image correspondences sets](../../data/obj_img_cors.html).

    misc/cat_obj_img_cors in1.json in2.json out.json [1/2]
    
Possible combinations:

- _in1_ = 1-object-1-image, and _in2_ = 1-object-1-image
- _in1_ = 1-object-2-image, and _in2_ = 1-object-2-image
- _in1_ = 1-object-2-image, and _in2_ = 1-object-1-image: third argument indicates which to take from _in1_
- _in1_ = 1-object-1-image, and _in2_ = 1-object-2-image: third argument indicates which to take from _in2_
