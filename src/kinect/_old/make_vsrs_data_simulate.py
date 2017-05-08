#!/usr/local/bin/python
#TODO convert

import sys, os, json, math


def usage_fail():
	print("usage: {} parameters.json [nocameras]\n".format(sys.argv[0]))
	sys.exit(1)


def clamp(minvalue, value, maxvalue):
    return max(minvalue, min(value, maxvalue))
			

if len(sys.argv) <= 1: usage_fail()
parameters_filename = sys.argv[1]
nocameras = False
if len(sys.argv) > 2: nocameras = (sys.argv[2] == "nocameras")

cross_x_offset = 0

with open(parameters_filename) as f:
	parameters = json.load(f)
arrangement = parameters["arrangement"]
raw_arrangement = parameters["arrangement"]["kinect_raw"]

camera_names = None
if not nocameras:
	cameras_filename = os.path.join(os.path.dirname(parameters_filename), arrangement["cameras_filename"])
	with open(cameras_filename) as f:
		cameras = json.load(f)
		
	camera_names = set()
	for cam in cameras:
		camera_names.add(cam["name"])


def camera_exists(name):
	return (camera_names is None) or (name in camera_names)

errors = list()

def process_view(x_index, y_index):	
	if y_index is not None:
		raw_y_index = y_index;
		if "y_index_factor" in raw_arrangement: raw_y_index = math.trunc(raw_y_index * raw_arrangement["y_index_factor"])
		if "y_index_offset" in raw_arrangement: raw_y_index = raw_y_index + raw_arrangement["y_index_offset"]


	x_out_index = x_index;
	x_in_index = x_out_index + cross_x_offset
	x_in_index = clamp(arrangement["x_index_range"][0], x_in_index, arrangement["x_index_range"][1])

	raw_x_in_index = x_in_index;
	if "x_index_factor" in raw_arrangement: raw_x_in_index = math.trunc(raw_x_in_index * raw_arrangement["x_index_factor"])
	if "x_index_offset" in raw_arrangement: raw_x_in_index = raw_x_in_index + raw_arrangement["x_index_offset"]

	raw_x_out_index = x_in_index;
	if "x_index_factor" in raw_arrangement: raw_x_out_index = math.trunc(raw_x_out_index * raw_arrangement["x_index_factor"])
	if "x_index_offset" in raw_arrangement: raw_x_out_index = raw_x_out_index + raw_arrangement["x_index_offset"]

	def format_filename(filename):
		if y_index is not None: return filename.format(x=x_out_index, y=y_index)
		else: return filename.format(x=x_out_index)
	def format_raw_filename(filename):
		if y_index is not None: return filename.format(x=raw_x_out_index, y=raw_y_index)
		else: return filename.format(x=raw_x_out_index)
	def format_raw_in_filename(filename):
		if y_index is not None: return filename.format(x=raw_x_in_index, y=raw_y_index)
		else: return filename.format(x=raw_x_in_index)
	def format_tmp_filename(filename):
		if y_index is not None: return filename.format(str(x_out_index) + "_" + str(y_index))
		else: return filename.format(x_in_index)
	
	texture_filename = os.path.join(os.path.dirname(parameters_filename), format_raw_filename(raw_arrangement["texture_filename_format"]))
	depth_filename = os.path.join(os.path.dirname(parameters_filename), format_raw_filename(raw_arrangement["depth_filename_format"]))

	disparity_yuv_filename = os.path.join(os.path.dirname(parameters_filename), format_filename(arrangement["depth_filename_format"]))
	texture_yuv_filename = os.path.join(os.path.dirname(parameters_filename), format_filename(arrangement["texture_filename_format"]))

	in_camera_name = arrangement["camera_name_format"].format(x=x_in_index, y=y_index)

	print "view (x={}, y={}):\n   raw texture: {}\n     raw depth: {}\n\n   yuv texture: {}\n     yuv depth: {}\n\n   camera: {}\n".format(
		x_index,
		y_index,
		texture_filename,
		depth_filename,
		texture_yuv_filename,
		disparity_yuv_filename,
		in_camera_name
	)
	
	if not os.path.isfile(texture_filename): errors.append("texture {} does not exist".format(texture_filename))
	elif os.path.getsize(texture_filename) == 0: errors.append("texture {} is empty file".format(texture_filename))
	if not os.path.isfile(depth_filename): errors.append("depth {} does not exist".format(depth_filename))
	elif os.path.isfile(depth_filename) == 0: errors.append("depth {} is empty file".format(depth_filename))
	if not camera_exists(in_camera_name): errors.append("camera {} does not exist".format(in_camera_name))

if __name__ == '__main__':
	x_index_step = arrangement["x_index_range"][2] if len(arrangement["x_index_range"]) == 3 else 1 
	x_index_range = range(arrangement["x_index_range"][0], arrangement["x_index_range"][1]+1, x_index_step)
	total_count = (arrangement["x_index_range"][1] - arrangement["x_index_range"][0] + 1)//x_index_step

	if "y_index_range" in arrangement:
		y_index_step = arrangement["y_index_range"][2] if len(arrangement["y_index_range"]) == 3 else 1 
		y_index_range = range(arrangement["y_index_range"][0], arrangement["y_index_range"][1]+1, y_index_step)
		total_count = total_count * (arrangement["y_index_range"][1] - arrangement["y_index_range"][0] + 1)//y_index_step
		indices = [(x, y) for y in y_index_range for x in x_index_range]	
	else:
		indices = [(x, None) for x in x_index_range]	
	
	for xy in indices: process_view(*xy)
	
	print "{} errors".format(len(errors))
	for err in errors:
		print "- {}".format(err)
