#!/usr/local/bin/python

import sys, os, json, math
from shutil import copyfile


if __name__ == '__main__':
	def usage_fail():
		print("usage: {} parameters.json out_directory\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 2: usage_fail()
	parameters_filename = sys.argv[1]
	output_directory = sys.argv[2]
		
	with open(parameters_filename) as f:
		parameters = json.load(f)
	arrangement = parameters["arrangement"]
	raw_arrangement = parameters["arrangement"]["kinect_raw"]

	y_index_step = arrangement["y_index_range"][2] if len(arrangement["y_index_range"]) == 3 else 1
	y_range = range(arrangement["y_index_range"][0], arrangement["y_index_range"][1]+1, y_index_step)
	y_length = arrangement["y_index_range"][1] - arrangement["y_index_range"][0]

	x_index_step = arrangement["x_index_range"][2] if len(arrangement["x_index_range"]) == 3 else 1
	x_range = range(arrangement["x_index_range"][0], arrangement["x_index_range"][1]+1, x_index_step)
	x_length = arrangement["x_index_range"][1] - arrangement["x_index_range"][0]
	
	def to_indices(x, y):
		x_index = math.trunc(x * x_length)
		x_index = math.trunc(x_index / x_index_step) * x_index_step
		x_index = x_index + arrangement["x_index_range"][0]
		assert(x_index >= arrangement["x_index_range"][0])
		assert(x_index <= arrangement["x_index_range"][1])
		
		y_index = math.trunc(y * y_length)
		y_index = math.trunc(y_index / y_index_step) * y_index_step
		y_index = y_index + arrangement["y_index_range"][0]
		assert(y_index >= arrangement["y_index_range"][0])
		assert(y_index <= arrangement["y_index_range"][1])

		return (x_index, y_index)

	n_count = 300
	for n in range(n_count+1):
		t = (n / float(n_count)) * 2 * math.pi
		x = (1 + math.cos(t)) / 2
		y = (1 + math.sin(2*t)) / 2
		x_index, y_index = to_indices(x, y)
		
		raw_x_index = x_index;
		if "x_index_factor" in raw_arrangement: raw_x_index = math.trunc(raw_x_index * raw_arrangement["x_index_factor"])
		if "x_index_offset" in raw_arrangement: raw_x_index = raw_x_index + raw_arrangement["x_index_offset"]

		raw_y_index = y_index;
		if "y_index_factor" in raw_arrangement: raw_y_index = math.trunc(raw_y_index * raw_arrangement["y_index_factor"])
		if "y_index_offset" in raw_arrangement: raw_y_index = raw_y_index + raw_arrangement["y_index_offset"]

		texture_filename = os.path.join(os.path.dirname(parameters_filename), raw_arrangement["texture_filename_format"].format(x=raw_x_index, y=raw_y_index))
		texture_copy_filename = os.path.join(output_directory, "{:05d}.png".format(n))

		copyfile(texture_filename, texture_copy_filename)

