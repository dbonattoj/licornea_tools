# kinect/checkerboard\_depth\_parallel

Estimates depth of checkerboard using simple pin-hole model.

    kinect/checkerboard_depth_parallel cols rows square_width ir_intr.json
    
Detects checkerboard in IR image. Estimates the depth of the checkerboard, **only if** it is held parallel to the Kinect IR camera plane.
