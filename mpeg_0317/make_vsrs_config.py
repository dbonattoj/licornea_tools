#!/usr/bin/python

import sys, os

'''
Script which generates VSRS configuration file for this data set:
M40293 -  ULB High Density 2D Camera Array data set, version 1

23 March 2017



USAGE
=====
	make_vsrs_config.py   band   left_row left_col   virtual_row virtual_col  right_row right_col   output_virtual.yuv output_config.txt

Where band is 1, 2 or 3. Generates configuration file for synthesizing view (virtual_row,virtual_col) out of
(left_row,left_col) and (right_row,right_col). These three view 2-indices must be in the range of the specified band:

band 1:
	row must be in (1, 3, ..., 101)
	col must be in (1, 3, ..., 849)

band 2:
	row must be in (101, 102, ..., 201)
	col must be in (1, 2, ..., 851)

band 3:
	row must be in (201, 203, ..., 301)
	col must be in (1, 3, ..., 849)

The generated config file output_config.txt can then be used with VSRS like:

	./ViewSyn output_config.txt

VSRS will generate the output view in the YUV file output_virtual.yuv.
It can be converted to a PNG file using ffmpeg with

	ffmpeg -f rawvideo -vcodec rawvideo -s 1928x1080 -pix_fmt yuv420p -i output_virtual.yuv -frames 1 output_virtual.png


DIRECTORY STRUCTURE
===================

The script expects the following directory structure for the YUV file and camera parameter files:

+ 3DLicornea_A1/
	+ disparity/
		+ Band1_0001-0101_step2mm/
			+ files/
		+ Band2_0101-0201_step1mm/
			+ files/
		+ Band3_0201-0301_step2mm/
			+ files/
	+ textures/
		+ Band1_0001-0101_step2mm/
			+ files/
		+ Band2_0101-0201_step1mm/
			+ files/
		+ Band3_0201-0301_step2mm/
			+ files/
	+ parameters/
		+ Band1_0001-0101_step2mm/
			+ vsrs_all.txt
		+ Band2_0101-0201_step1mm/
			+ vsrs_all.txt
		+ Band3_0201-0301_step2mm/
			+ vsrs_all.txt

It corresponds to the directory structure of the set on the MPEG servers.
Files in the zip archives must be decompressed and put (all together) into the files/ directory for that band.

YUV files in the files/ directories all have names like cam_01010401.yuv
where 0101 is the row index, and 0401 is the column index.

vsrs_all.txt files contain camera parameters in VSRS format, for all the views in one band.


The script must be launched from the directory containing 3DLicornea_A1/.
That is, ./3DLicornea_A1 must be a valid relative path when the script executes.
VSRS must also be launched from the same directory, because the generated configuration file will contain relative paths.


The template below can be modified, to change the configuration file that will be generated:
'''


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


################################################################################


def usage_fail():
	print("usage: {}   band   left_row left_col   virtual_row virtual_col  right_row right_col   output_virtual.yuv output_config.txt\n".format(sys.argv[0]))
	sys.exit(1)

def fail(msg):
	print(msg)
	sys.exit(1)

def no_file_fail(path):
	print("file does not exist: {}".format(path))
	sys.exit(1)

def check_file_existence(filename):
	if not os.path.isfile(filename): fail("no such file: {}".format(filename))

def check_idx_range(band, row, col, name):
	if band == 1:
		if not(row >= 1 and row <= 101 and row % 2 != 0):
			fail("{}: for band 1: row index {} must be in (1, 3, ..., 101)".format(name, row))
		elif not(col >= 1 and col <= 849 and col % 2 != 0):
			fail("{}: for band 1 row {}: column index {} must be in (1, 3, ..., 849)".format(name, row, col))
	
	elif band == 2:
		if not(row >= 101  and row <= 201):
			fail("{}: for band 2: row index {} must be in (101, 2, ..., 201)".format(name, row))
		elif not(col >= 1 and col <= 851):
			fail("{}: for band 2 row {}: column index {} must be in (1, 2, ..., 851)".format(name, row, col))

	elif band == 3:		
		if not(row > 201  and row <= 301 and row % 2 != 0):
			fail("{}: for band 3: row index {} must be in (201, 203, ..., 301)".format(name, row))
		elif not(col >= 1 and col <= 849 and col % 2 != 0):
			fail("{}: for band 3 row {}: column index {} must be in (1, 3, ..., 849)".format(name, row, col))

def band_dirname(band, section):
	root_dirname = "3DLicornea_A1"
	if band == 1:   band_dirname = "Band1_0001-0101_step2mm"
	elif band == 2: band_dirname = "Band2_0101-0201_step1mm"
	elif band == 3: band_dirname = "Band3_0201-0301_step2mm"
	return os.path.join(root_dirname, section, band_dirname)

def view_name(row, col):
	return "cam_{:04d}{:04d}".format(row, col)

def data_filenames(band, row, col):
	yuv_texture_filename = os.path.join(band_dirname(band, "textures"), "files/{}.yuv".format(view_name(row, col)))
	yuv_disparity_filename = os.path.join(band_dirname(band, "disparity"), "files/{}.yuv".format(view_name(row, col)))
	check_file_existence(yuv_texture_filename)
	check_file_existence(yuv_disparity_filename)

	return yuv_texture_filename, yuv_disparity_filename

if __name__ == '__main__':
	if len(sys.argv) <= 9: usage_fail()
	band = int(sys.argv[1])
	left_row_idx = int(sys.argv[2])
	left_col_idx = int(sys.argv[3])
	virtual_row_idx = int(sys.argv[4])
	virtual_col_idx = int(sys.argv[5])
	right_row_idx = int(sys.argv[6])
	right_col_idx = int(sys.argv[7])
	output_virtual_filename = sys.argv[8]
	output_config_filename = sys.argv[9]

	if band != 1 and band != 2 and band != 3: fail("band must be 1, 2 or 3")
	check_idx_range(band, left_row_idx, left_col_idx, "left")
	check_idx_range(band, virtual_row_idx, virtual_col_idx, "virtual")
	check_idx_range(band, right_row_idx, right_col_idx, "right")

	left_yuv_texture_filename, left_yuv_disparity_filename = data_filenames(band, left_row_idx, left_col_idx)
	right_yuv_texture_filename, right_yuv_disparity_filename = data_filenames(band, right_row_idx, right_col_idx)

	cameras_filename = os.path.join(band_dirname(band, "parameters"), "vsrs_all.txt")
	check_file_existence(cameras_filename)
	
	left_camera_name = view_name(left_row_idx, left_col_idx)
	virtual_camera_name = view_name(virtual_row_idx, virtual_col_idx)
	right_camera_name = view_name(right_row_idx, right_col_idx)

	config = config_tmp.format(
		width=1920,
		height=1080,
		z_near=600.0,
		z_far=1600.0,
		cam_param=cameras_filename,
		cam_left=left_camera_name,
		cam_virtual=virtual_camera_name,
		cam_right=right_camera_name,
		texture_left=left_yuv_texture_filename,
		texture_right=right_yuv_texture_filename,
		depth_left=left_yuv_disparity_filename,
		depth_right=right_yuv_disparity_filename,
		output=output_virtual_filename
	)

	with open(output_config_filename, 'w') as f:
		print >>f, config


