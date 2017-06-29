#!/usr/local/bin/python
from pylib import *

config_tmp = """
#================ Input Parameters ================
DepthType                      1                        # 0...Depth from camera, 1...Depth from the origin of 3D space
SourceWidth                    {width}                      # Input frame width
SourceHeight                   {height}                      # Input frame height
StartFrame                     0						# Starting frame #
TotalNumberOfFrames            1                      # Total number of input frames 
LeftNearestDepthValue          {z_near}              # Nearest depth value of left image from camera or the origin of 3D space
LeftFarthestDepthValue         {z_far}             # Farthest depth value of left image from camera or the origin of 3D space
RightNearestDepthValue         {z_near}              # Nearest depth value of right image from camera or the origin of 3D space
RightFarthestDepthValue        {z_far}             # Farthest depth value of right image from camera or the origin of 3D space
CameraParameterFile            {cam_param}         # Name of text file which includes real and virtual camera parameters
LeftCameraName                 {cam_left}               # Name of real left camera
VirtualCameraName              {cam_virtual}             # Name of virtual camera
RightCameraName                {cam_right}               # Name of real right camera
LeftViewImageName              {texture_left}                 # Name of left input video
RightViewImageName             {texture_right}                 # Name of right input video
LeftDepthMapName               {depth_left}           # Name of left depth map video
RightDepthMapName              {depth_right}           # Name of right depth map video
OutputVirtualViewImageName     {output}      # Name of output virtual view video

ColorSpace                     0                        # 0...YUV, 1...RGB
Precision                      2                        # 1...Integer-pel, 2...Half-pel, 4...Quater-pel
Filter                         1                        # 0...(Bi)-linear, 1...(Bi)-Cubic, 2...MPEG-4 AVC

BoundaryNoiseRemoval		   		 1						# Boundary Noise Removal: Updated By GIST

SynthesisMode                  0                        # 0...General, 1...1D parallel

#---- General mode ------
ViewBlending                  0                        # 0...Blend left and right images, 1...Not Blend

#---- 1D mode      ------
#---- In this example, all parameters below are commented and default values will be taken ----
#SplattingOption       2                 # 0: disable; 1: Enable for all pixels; 2: Enable only for boundary pixels. Default: 2
#BoundaryGrowth        40                # A parameter to enlarge the boundary area with SplattingOption = 2. Default: 40
#MergingOption         2                 # 0: Z-buffer only; 1: Averaging only; 2: Adaptive merging using Z-buffer and averaging. Default: 2
#DepthThreshold        75                # A threshold is only used with MergingOption = 2. Range: 0 ~ 255. Default: 75
#HoleCountThreshold    30                # A threshold is only used with MergingOption = 2. Range: 0 ~ 49. Default: 30
"""

def main(parameters_filename, cameras_filename, left_idx, virtual_idx, right_idx, output_virtual_filename, output_config_filename):
	datas = Dataset(parameters_filename)
	
	par = datas.parameters
	vsrs_par = datas.group("vsrs").parameters()
	
	left_view = datas.view(left_idx).group_view("vsrs")
	virtual_view = datas.view(virtual_idx).group_view("vsrs")
	right_view = datas.view(right_idx).group_view("vsrs")
	
	texture_left_filename = left_view.image_filename()
	texture_right_filename = right_view.image_filename()
	depth_left_filename = left_view.depth_filename()
	depth_right_filename = right_view.depth_filename()

	print texture_left_filename

	assert(os.path.isfile(cameras_filename))
	assert(os.path.isfile(texture_left_filename))
	assert(os.path.isfile(texture_right_filename))
	assert(os.path.isfile(depth_left_filename))
	assert(os.path.isfile(depth_right_filename))

	config = config_tmp.format(
		width=par["width"],
		height=par["height"],
		z_near=vsrs_par["z_near"],
		z_far=vsrs_par["z_far"],
		cam_param=cameras_filename,
		cam_left=left_view.camera_name(),
		cam_virtual=virtual_view.camera_name(),
		cam_right=right_view.camera_name(),
		texture_left=texture_left_filename,
		texture_right=texture_right_filename,
		depth_left=depth_left_filename,
		depth_right=depth_right_filename,
		output=output_virtual_filename
	)

	with open(output_config_filename, 'w') as f:
		print >>f, config


# TODO support 2D dataset

if __name__ == '__main__':
	def usage_fail():
		print("usage: {} dataset_parameters.json cameras.txt left_idx virtual_idx right_idx output_virtual.yuv output_config.txt\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 7: usage_fail()
	parameters_filename = sys.argv[1]
	cameras_filename = sys.argv[2]
	left_idx = int(sys.argv[3])
	virtual_idx = int(sys.argv[4])
	right_idx = int(sys.argv[5])
	output_virtual_filename = sys.argv[6]
	output_config_filename = sys.argv[7]

	main(parameters_filename, cameras_filename, left_idx, virtual_idx, right_idx, output_virtual_filename, output_config_filename)
