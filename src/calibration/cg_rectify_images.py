#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

datas = None
homographies = None
R_filename = None
intrinsics_filename = None

image = True
depth = True

overwrite_image = False
overwrite_depth = False


border = {
	"top" : 0,
	"left" : 0,
	"bottom" : 0,
	"right" : 0
}

def process_view(x, y):	
	if verbose: print "view x={}, y={}".format(x, y)
	
	view = datas.view(x, y)
	rectified_view = view.rectified()
	homography = homographies[encode_view_index(x, y)]
	
	with temporary_in_json(homography) as tmp_homography, \
		temporary_in_json(border) as tmp_border, \
		temporary_file("png") as tmp_unrotated_depth:
		
		if image:
			in_image_filename = view.image_filename()
			assert os.path.isfile(in_image_filename)
			out_rectified_image_filename = rectified_view.image_filename()
			
			if overwrite_image or not os.path.isfile(out_rectified_image_filename):
				call_tool("misc/apply_homography", [
					tmp_homography.filename,
					in_image_filename,
					out_rectified_image_filename,
					"texture",
					tmp_border.filename
				])
		
		if depth:
			in_depth_filename = view.depth_filename()
			assert os.path.isfile(in_depth_filename)
			out_rectified_depth_filename = rectified_view.depth_filename()
		
			if overwrite_depth or not os.path.isfile(out_rectified_depth_filename):
				call_tool("calibration/cg_unrotate_depth_map", [
					view.depth_filename(),
					intrinsics_filename,
					R_filename,
					tmp_unrotated_depth.filename,
				])
				call_tool("misc/apply_homography", [
					tmp_homography.filename,
					tmp_unrotated_depth.filename,
					out_rectified_depth_filename,
					"depth",
					tmp_border.filename
				])


def usage_fail():
	print("usage: {} dataset_parameters.json intrinsics.json R.json homographies.json\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	if len(sys.argv) <= 4: usage_fail()
	parameters_filename = sys.argv[1]
	intrinsics_filename = sys.argv[2]
	R_filename = sys.argv[3]
	homographies_filename = sys.argv[4];

	datas = Dataset(parameters_filename)
	with open(homographies_filename, 'r') as f:
		homographies = json.load(f)
		
	grp_par = datas.group_parameters("rectified")
	keys = ["top", "left", "bottom", "right"];
	if "border" in grp_par:
		bord_par = grp_par["border"]
		for key in keys:
			if key in bord_par: border[key] = bord_par[key]
	
	indices = [(x, y) for y in datas.y_indices() for x in datas.x_indices()]
	
	batch_process(process_view, indices)
	
	print "done."

