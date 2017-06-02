#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

simulate = False

datas = None
densify_method = None
internal_parameters_filename = None
reprojection_parameters_filename = None

image = True
depth = True

overwrite_image = False
overwrite_depth = False

def process_view(x, y):	
	if verbose: print "view x={}, y={}".format(x, y)
	
	view = datas.view(x, y)
	raw_view = view.kinect_raw()
	
	if image:
		out_image_filename = view.image_filename()
		in_image_filename = raw_view.image_filename()		
		assert os.path.isfile(in_image_filename)
		
		if overwrite_image or not os.path.isfile(out_image_filename):
			if verbose: print "copying image {} -> {}".format(in_image_filename, out_image_filename)
			if not simulate:
				shutil.copyfile(in_image_filename, out_image_filename)
		
	if depth:
		out_depth_filename = view.depth_filename("-")
		out_mask_filename = view.mask_filename("-")
		in_depth_filename = raw_view.depth_filename()
		assert os.path.isfile(in_depth_filename)
			
		if overwrite_depth or not os.path.isfile(out_depth_filename):
			if verbose: print "reprojecting depth {} -> {}".format(in_depth_filename, out_depth_filename)
			if not simulate:
				call_tool("kinect/depth_reprojection", [
					in_depth_filename,
					out_depth_filename,
					out_mask_filename,
					reprojection_parameters_filename,
					densify_method
				])
			

def usage_fail():
	print("usage: {} dataset_parameters.json densify_method [simulate]\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	if len(sys.argv) <= 2: usage_fail()
	parameters_filename = sys.argv[1]
	densify_method = sys.argv[2];
	if len(sys.argv) > 3:
		if sys.argv[3] == "simulate": simulate = True
		else: usage_fail()

	if simulate:
		parallel = False
		verbose = True

	datas = Dataset(parameters_filename)
	reprojection_parameters_filename = datas.filepath(datas.parameters["kinect_raw"]["kinect_reprojection_parameters_filename"])
	
	indices = [(x, y) for y in datas.y_indices() for x in datas.x_indices()]
	
	batch_process(process_view, indices)
	
	print "done."

