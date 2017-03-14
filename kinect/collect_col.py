#!/usr/local/bin/python

import sys, os, json
from shutil import copyfile

if __name__ == '__main__':
	def usage_fail():
		print("usage: {} parameters.json raw_data_directory x_idx out_directory\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 4: usage_fail()
	parameters_filename = sys.argv[1]
	raw_data_directory = sys.argv[2]
	x_index = int(sys.argv[3])
	output_directory = sys.argv[4]
		
	with open(parameters_filename) as f:
		parameters = json.load(f)
	arrangement = parameters["arrangement"]
	raw_arrangement = parameters["arrangement"]["kinect_raw"]

	y_range = range(arrangement["y_index_range"][0], arrangement["y_index_range"][1]+1)

	raw_x_index = x_index;
	if "x_index_factor" in raw_arrangement: raw_x_index = raw_x_index * raw_arrangement["x_index_factor"]
	if "x_index_offset" in raw_arrangement: raw_x_index = raw_x_index + raw_arrangement["x_index_offset"]

	for y_index in y_range:
		raw_y_index = y_index;
		if "y_index_factor" in raw_arrangement: raw_y_index = raw_y_index * raw_arrangement["y_index_factor"]
		if "y_index_offset" in raw_arrangement: raw_y_index = raw_y_index + raw_arrangement["y_index_offset"]

		texture_filename = os.path.join(raw_data_directory, raw_arrangement["texture_filename_format"].format(x=raw_x_index, y=raw_y_index))
		texture_copy_filename = os.path.join(output_directory, "{:04d}.png".format(y_index))

		copyfile(texture_filename, texture_copy_filename)

