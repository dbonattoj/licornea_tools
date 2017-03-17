#!/usr/local/bin/python

import sys, os, json, subprocess

tools_directory = "."
intrinsics_filename = "../data/kinect_internal_intrinsics.json"
verbose = True

def call_tool(tool, args):
	full_args = [os.path.join(tools_directory, tool)] + args
	if verbose:
		print "calling {}".format(tool)
		subprocess.check_call(full_args)
	else:
		try:
			subprocess.check_output(full_args)
		except CalledProcessError as err:
			print "{} failed. output:\n{}".format(tool, err.output)
			raise


def usage_fail():
	print("usage: {} parameters.json output.png output_mask.png method x_out_idx x_in_idx [y_out_idx y_in_idx]\n".format(sys.argv[0]))
	sys.exit(1)


if __name__ == '__main__':
	if len(sys.argv) <= 6: usage_fail()
	parameters_filename = sys.argv[1]
	output_filename = sys.argv[2]
	output_mask_filename = sys.argv[3]
	method = sys.argv[4]
	x_out_idx = int(sys.argv[5])
	x_in_idx = int(sys.argv[6])
	y_out_idx = None
	y_in_idx = None
	if len(sys.argv) > 7:y_out_idx = int(sys.argv[7])
	if len(sys.argv) > 8: y_in_idx = int(sys.argv[8])
	else: y_in_idx = y_out_idx;

	with open(parameters_filename) as f:
		parameters = json.load(f)
	arrangement = parameters["arrangement"]
	raw_arrangement = parameters["arrangement"]["kinect_raw"]

	raw_x_in_idx = x_in_idx;
	if "x_index_factor" in raw_arrangement: raw_x_in_idx = raw_x_in_idx * raw_arrangement["x_index_factor"]
	if "x_index_offset" in raw_arrangement: raw_x_in_idx = raw_x_in_idx + raw_arrangement["x_index_offset"]
	
	if y_out_idx is not None:
		raw_y_in_idx = y_in_idx;
		if "y_index_factor" in raw_arrangement: raw_y_in_idx = raw_y_in_idx * raw_arrangement["y_index_factor"]
		if "y_index_offset" in raw_arrangement: raw_y_in_idx = raw_y_in_idx + raw_arrangement["y_index_offset"]


	if "y_index_range" in arrangement:
		if y_out_idx is None: raise Exception("for 2D set, y_index must be specified")
		point_cloud_filename = os.path.join(os.path.dirname(output_filename), "point_cloud_{}.ply".format(str(x_in_idx) + "_" + str(y_in_idx)))
		in_camera_name = arrangement["camera_name_format"].format(x=x_in_idx, y=y_in_idx)
		out_camera_name = arrangement["camera_name_format"].format(x=x_out_idx, y=y_out_idx)
		input_filename = raw_arrangement["depth_filename_format"].format(x=raw_x_in_idx, y=raw_y_in_idx)		
	else:
		point_cloud_filename = os.path.join(os.path.dirname(output_filename), "point_cloud_{}.ply".format(str(x_in_idx)))
		in_camera_name = arrangement["camera_name_format"].format(x=x_in_idx)
		out_camera_name = arrangement["camera_name_format"].format(x=x_out_idx)
		input_filename = raw_arrangement["depth_filename_format"].format(x=raw_x_in_idx)
		
	if not os.path.isfile(input_filename):
		call_tool("kinect/depth_point_cloud", [
			os.path.join(os.path.dirname(parameters_filename), input_filename),
			point_cloud_filename,
			intrinsics_filename,
			"color"
		])
		
	call_tool("kinect/point_cloud_reprojection", [
		point_cloud_filename,
		output_filename,
		output_mask_filename,
		intrinsics_filename,
		method,
		os.path.join(os.path.dirname(parameters_filename), arrangement["cameras_filename"]),
		in_camera_name,
		out_camera_name
	])
	
	os.remove(point_cloud_filename)
	
