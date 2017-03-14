#!/usr/local/bin/python

import sys, os, json

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
	with open(parameters_filename) as f:
		parameters = json.load(f)

	config = config_tmp.format(
		width=parameters["texture"]["width"],
		height=parameters["texture"]["height"],
		z_near=parameters["depth"]["z_near"],
		z_far=parameters["depth"]["z_far"],
		cam_param=cameras_filename,
		cam_left=parameters["arrangement"]["camera_name_format"].format(x=left_idx),
		cam_virtual=parameters["arrangement"]["camera_name_format"].format(x=virtual_idx),
		cam_right=parameters["arrangement"]["camera_name_format"].format(x=right_idx),
		texture_left=os.path.join(os.path.dirname(parameters_filename), parameters["arrangement"]["texture_filename_format"].format(x=left_idx)),
		texture_right=os.path.join(os.path.dirname(parameters_filename), parameters["arrangement"]["texture_filename_format"].format(x=right_idx)),
		depth_left=os.path.join(os.path.dirname(parameters_filename), parameters["arrangement"]["depth_filename_format"].format(x=left_idx)),
		depth_right=os.path.join(os.path.dirname(parameters_filename), parameters["arrangement"]["depth_filename_format"].format(x=right_idx)),
		output=output_virtual_filename
	)

	with open(output_config_filename, 'w') as f:
		print >>f, config



if __name__ == '__main__':
	def usage_fail():
		print("usage: {} parameters.json cameras.txt left_idx virtual_idx right_idx output_virtual.yuv output_config.txt\n".format(sys.argv[0]))
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
