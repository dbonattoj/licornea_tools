#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

simulate = False
dataset_group = ""

image = True
depth = True

overwrite_image = True
overwrite_depth = True

def process_view(x, y):	
	if verbose: print "view x={}, y={}".format(x, y)
	
	view = dataset.view(x, y).group_view(dataset_group)
	vsrs_view = view.group_view("vsrs")
	
	if image:
		out_yuv_image_filename = vsrs_view.image_filename()
		in_image_filename = view.image_filename()
		assert os.path.isfile(in_image_filename)
		if overwrite_image or not os.path.isfile(out_yuv_image_filename):
			if verbose: print "converting image to yuv {} -> {}".format(in_image_filename, out_image_filename)
			png2yuv(in_image_filename, out_yuv_image_filename)
		
	if depth:
		out_yuv_disparity_filename = vsrs_view.depth_filename()
		in_depth_filename = view.depth_filename()
		assert os.path.isfile(in_depth_filename)
			
		if overwrite_depth or not os.path.isfile(out_yuv_disparity_filename):
			if verbose: print "converting depth to yuv disparity {} -> {}".format(in_depth_filename, out_depth_filename)
			if not simulate:
				call_tool("vsrs/vsrs_disparity", [
					in_depth_filename,
					out_yuv_disparity_filename,
					vsrs_view.local_parameter("z_near"),
					vsrs_view.local_parameter("z_far"),
					8
				])



def usage_fail():
	print("usage: {} parameters.json [simulate] [dataset_group]\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	if len(sys.argv) <= 1: usage_fail()
	parameters_filename = sys.argv[1]
	if len(sys.argv) > 2:
		if sys.argv[2] == "simulate": simulate = True
	if len(sys.argv) > 3:
		dataset_group = sys.argv[3]

	if simulate:
		print("simulation mode")
		parallel = False
		verbose = True

	dataset = Dataset(parameters_filename)
	
	indices = [idx for idx in datas.indices()]
	
	batch_process(process_view, indices)
	
	print "done."

