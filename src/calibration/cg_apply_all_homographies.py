#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

datas = None
homographies = None

def process_view(x, y):	
	if verbose: print "view x={}, y={}".format(x, y)
	
	view = datas.view(x, y)
	rectified_view = view.rectified()
	homography = homographies[encode_view_index(x, y)]
	
	with temporary_json(homography) as tmp_homography:
		call_tool("misc/apply_homography", [
			tmp_homography.filename,
			view.image_filename(),
			rectified_view.image_filename(),
			"texture"
		])
	
def usage_fail():
	print("usage: {} dataset_parameters.json homographies.json\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	if len(sys.argv) <= 2: usage_fail()
	parameters_filename = sys.argv[1]
	homographies_filename = sys.argv[2];

	datas = Dataset(parameters_filename)
	with open(homographies_filename, 'r') as f:
		homographies = json.load(f)
	
	indices = [(x, y) for y in datas.y_indices() for x in datas.x_indices()]
	
	batch_process(process_view, indices)
	
	print "done."

