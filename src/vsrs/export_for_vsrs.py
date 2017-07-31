#!/usr/local/bin/python
from pylib.config import *
from pylib.dataset import *
from pylib.utility import *
import sys, os, json, shutil

##########

def export_view(datag, idx, image=True, depth=True, overwrite_image=True, overwrite_depth=True, simulate=False):
	if verbose: print "exporting to vsrs view {}".format(encode_view_index(idx))
		
	view = datag.view(*idx)
	vsrs_view = view.group_view("vsrs")
	
	if image:
		out_yuv_image_filename = vsrs_view.image_filename()
		in_image_filename = view.image_filename()
		assert os.path.isfile(in_image_filename)
		if overwrite_image or not os.path.isfile(out_yuv_image_filename):
			if verbose: print "converting image to yuv {} -> {}".format(in_image_filename, out_yuv_image_filename)
			if not simulate:
				call_tool("misc/yuv_export", [
					in_image_filename,
					out_yuv_image_filename,
					"ycbcr420"
				])
		
	if depth:
		out_yuv_disparity_filename = vsrs_view.depth_filename()
		in_depth_filename = view.depth_filename()
		assert os.path.isfile(in_depth_filename)
			
		if overwrite_depth or not os.path.isfile(out_yuv_disparity_filename):
			if verbose: print "converting depth to yuv disparity {} -> {}".format(in_depth_filename, out_yuv_disparity_filename)
			if not simulate:
				call_tool("vsrs/vsrs_disparity", [
					in_depth_filename,
					out_yuv_disparity_filename,
					vsrs_view.local_parameter("z_near"),
					vsrs_view.local_parameter("z_far"),
					8
				])



def usage_fail():
	print("usage: {} dataset_parameters.json [simulate] [dataset_group]\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	image = True
	depth = True
	overwrite_image = False
	overwrite_depth = False
	simulate = False
	
	if len(sys.argv) <= 1: usage_fail()
	parameters_filename = sys.argv[1]
	if len(sys.argv) > 2:
		if sys.argv[2] == "simulate": simulate = True
	dataset_group = ""
	if len(sys.argv) > 3:
		dataset_group = sys.argv[3]

	if simulate:
		print("simulation mode")
		parallel = False
		verbose = True

	datas = Dataset(parameters_filename)
	datag = datas.group(dataset_group)

	def process_view(x, y):
		export_view(datag, (x, y), image=image, depth=depth, overwrite_image=overwrite_image, overwrite_depth=overwrite_depth, simulate=simulate)	
	indices = [idx for idx in datas.indices()]	
	batch_process(process_view, indices)
	
	print "done."

