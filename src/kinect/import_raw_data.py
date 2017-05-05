#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

simulate = False

datas = None
densify_method = None
internal_intrinsics_filename = None
color_intrinsics_filename = None

def process_view(x, y):	
	if verbose: print "view x={}, y={}".format(x, y)
	
	view = datas.view(x, y)
	raw_view = view.kinect_raw()
	
	out_image_filename = view.image_filename()
	out_depth_filename = view.depth_filename("-")
	out_mask_filename = view.mask_filename("-")
	
	in_image_filename = raw_view.image_filename()
	in_depth_filename = raw_view.depth_filename()
		
	assert os.path.isfile(in_image_filename)
	assert os.path.isfile(in_depth_filename)
	
	if not os.path.isfile(out_image_filename):
		if verbose: print "copying image {} -> {}".format(in_image_filename, out_image_filename)
		if not simulate:
			shutil.copyfile(in_image_filename, out_image_filename)
	
		#if verbose: print "undistorting image {}".format(out_image_filename)
		#if not simulate:
		#	call_tool("calibration/undistort", [
		#		out_image_filename,
		#		out_image_filename,
		#		color_intrinsics_filename,
		#		"texture"
		#	])
	
	if not os.path.isfile(out_depth_filename):
		if verbose: print "reprojecting depth {} -> {}".format(in_depth_filename, out_depth_filename)
		if not simulate:
			call_tool("kinect/depth_reprojection", [
				in_depth_filename,
				out_depth_filename,
				out_mask_filename,
				internal_intrinsics_filename,
				densify_method
			])

		#if verbose: print "undistorting depth {}".format(out_depth_filename)
		#if not simulate:
		#	call_tool("calibration/undistort", [
		#		out_depth_filename,
		#		out_depth_filename,
		#		color_intrinsics_filename,
		#		"depth"
		#	])
			

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

	if simulate: parallel = False

	datas = Dataset(parameters_filename)
	internal_intrinsics_filename = datas.filepath(datas.parameters["kinect_raw"]["kinect_internal_intrinsics_filename"])
	color_intrinsics_filename = datas.filepath(datas.parameters["kinect_raw"]["kinect_color_intrinsics_filename"])
	
	indices = [(x, y) for y in datas.y_indices() for x in datas.x_indices()]
	
	batch_process(process_view, indices)
	
	print "done."

