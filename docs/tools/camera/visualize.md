# camera/visualize

Generate 3D visualization of camera parameters.

    camera/visualize in_cameras.json out_view.ply world/view [scale=1.0]
   
Generate a PLY file with a visualization that shows the cameras in 3D space. It can be viewer for example in _MeshLab_.

If third parameter is `world`, it shows pyramids representing cameras in world space. The apex is the optical center, the base is the camera image plane. `scale` is the width of each model. It should be smaller than the distance between adjacent cameras, otherwise they will overlap.

If third parameter is `view`, it instead shows the camera pose in each camera's view space. That is, different coordinate systems, overlapped. This shows how the scene viewer from different cameras moves, instead of how the cameras move. It is a better visualization to estimate the quality of camera parameters for view synthesis: If the camera optical center moves, but then it reorients itself, the image will not change much. In the `world` visualization, the displaced optical center is very visible.

