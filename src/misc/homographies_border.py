#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

datas = None
homographies = None

def process_view(x, y):		
	homography = homographies[encode_view_index(x, y)]
	
	with temporary_in_json(homography) as tmp_homography, temporary_out_json() as tmp_border:
		call_tool("misc/homography_border", [
			tmp_homography.filename,
			datas.parameters["width"],
			datas.parameters["height"],
			"maximal_border",
			tmp_border.filename
		])
		return tmp_border.json()
	

def usage_fail():
	print("usage: {} dataset_parameters.json homographies.json out_border.json\n".format(sys.argv[0]))
	sys.exit(1)

if __name__ == '__main__':
	if len(sys.argv) <= 3: usage_fail()
	parameters_filename = sys.argv[1]
	homographies_filename = sys.argv[2];
	out_border_filename = sys.argv[3];

	datas = Dataset(parameters_filename)
	with open(homographies_filename, 'r') as f:
		homographies = json.load(f)
			
	indices = [(x, y) for y in datas.y_indices() for x in datas.x_indices()]
	
	borders = small_batch_process(process_view, indices)

	max_border = {}
	keys = ["top", "left", "bottom", "right"]
	for key in keys: max_border[key] = 0

	for border in borders:
		for key in keys:
			max_border[key] = max(max_border[key], border[key])
	
	export_json_file(max_border, out_border_filename)
	
	print "done."
