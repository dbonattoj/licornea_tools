#!/usr/local/bin/python
from pylib import *
from pylib.batch import *
from pylib.config import *
from pylib.dataset import *
from pylib.progress import *
from pylib.temporary import *
from pylib.utility import *
import sys, os, json, shutil

simulate = False

image = False
depth = True

overwrite_image = False
overwrite_depth = False


datas = None
densify_method = None
internal_parameters_filename = None
reprojection_parameters_filename = None


def process_view(x, y):	
	if verbose: print("view x={}, y={}".format(x, y))
	
	view = datas.view(x, y)
	raw_view = view.group_view("kinect_raw")
		
	if depth:
		out_depth_filename = view.depth_filename("-")
		out_mask_filename = view.mask_filename("-")
		in_depth_filename = raw_view.depth_filename()
		
		if not os.path.isfile(in_depth_filename):
			print("kinect_raw depth {} not found, skipping".format(in_depth_filename))
					
		elif overwrite_depth or not os.path.isfile(out_depth_filename):
			if verbose: print("reprojecting depth {} -> {}".format(in_depth_filename, out_depth_filename))
			if not simulate:
				#call_tool("misc/touch", [
				#	out_depth_filename,
				#	"no_create"
				#])
				call_tool("kinect/depth_reprojection", [
					in_depth_filename,
					out_depth_filename,
					out_mask_filename,
					reprojection_parameters_filename,
					densify_method
				])

	if image:
		out_image_filename = view.image_filename()
		in_image_filename = raw_view.image_filename()
		if not os.path.isfile(in_image_filename):
			print("kinect_raw image {} not found, skipping".format(in_image_filename))
			
		elif overwrite_image or not os.path.isfile(out_image_filename):
			if verbose: print("copying image {} -> {}".format(in_image_filename, out_image_filename))
			if not simulate:
				call_tool("misc/touch", [
					out_image_filename,
					"no_create"
				]) # workaround: create surrounding directories if needed
				shutil.copyfile(in_image_filename, out_image_filename)

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
	reprojection_parameters_filename = datas.filepath(datas.group("kinect_raw").parameters()["kinect_reprojection_parameters_filename"])
	
	indices = [idx for idx in datas.indices()]
	
	batch_process(process_view, indices)
	
	print("done.")

