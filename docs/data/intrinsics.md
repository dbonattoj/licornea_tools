# Camera intrinsics

Contains the camera matrix **K**, distortion coefficients, and image size of a camera. The distortion coefficients can be omitted if these is no distortion. The camera matrix should only consist of `fx, fy, cx, cy`. Some tools read only these four values from it.

Example intrinsics file for a Kinect's IR sensor:
```
{
    "K": [
        [366.882190, 0.0,        257.546861],
        [0.0,        368.335082, 209.158971],
        [0.0,        0.0,        1.0       ]
    ],
    "distortion": {
        "k1": 0.117712331292285,
        "k2": -0.342256099858845,
        "k3": 0.163986668762415,
        "p1": 0.0,
        "p2": 0.0
    },
    "height": 424,
    "width": 512
}
```

