# kinect/parallel\_wall

Determine if Kinect is placed parallel to a wall in the background.

    kinect/parallel_wall reprojection.json

Uses the reprojected depth map to test in measure real-time the orientation of the Kinect's color camera relative a flat surface in the background. [Reprojection parameters](../../data/reprojection.html) must be given.

If shows a GUI with the color image and reprojected depth map. Adjust the corner offset so that the square in the image is in a region where there is only the background wall visible in the image. Then the indicator shows the orientation. When it is at the center of the cross, the Kinect is placed parallel to the background. The indicator sensitivity can be adjusted.
