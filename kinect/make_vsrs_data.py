#!/usr/local/bin/python

import sys, os, subprocess, json, time

parallel = True
parallel_jobs = 4
tools_directory = "."
intrinsics_filename = "../data/kinect_internal_intrinsics.json"
verbose = False

if parallel:
	from joblib import Parallel, delayed


def usage_fail():
	print("usage: {} raw_data_directory output_directory parameters.json up/down\n".format(sys.argv[0]))
	sys.exit(1)


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
			

if len(sys.argv) <= 4: usage_fail()
raw_data_directory = sys.argv[1]
output_directory = sys.argv[2]
parameters_filename = sys.argv[3]
mode = sys.argv[4];
if mode != "up" and mode != "down": usage_fail()

with open(parameters_filename) as f:
	parameters = json.load(f)
arrangement = parameters["arrangement"]
raw_arrangement = parameters["arrangement"]["kinect_raw"]


z_near = parameters["depth"]["z_near"]
z_far = parameters["depth"]["z_far"]

total_count = None
start_time = None
done_count = None

def format_time(seconds):
	m, s = divmod(seconds, 60)
	h, m = divmod(m, 60)
	if h > 0: return "{}h {}min {}s".format(int(h), int(m), int(s))
	elif m > 0: return "{}min {}s".format(int(m), int(s))
	else: return "{}s".format(int(s))


def png2yuv(png, yuv):
	subprocess.check_call("ffmpeg -n -i {} -pix_fmt yuv420p {} > /dev/null 2>&1".format(png, yuv), shell=True)

def process_view(x_index, y_index):	
	print "view x={}, y={}".format(x_index, y_index)
	
	raw_x_index = x_index;
	if "x_index_factor" in raw_arrangement: raw_x_index = raw_x_index * raw_arrangement["x_index_factor"]
	if "x_index_offset" in raw_arrangement: raw_x_index = raw_x_index + raw_arrangement["x_index_offset"]
	
	if y_index is not None:
		raw_y_index = y_index;
		if "y_index_factor" in raw_arrangement: raw_y_index = raw_y_index * raw_arrangement["y_index_factor"]
		if "y_index_offset" in raw_arrangement: raw_y_index = raw_y_index + raw_arrangement["y_index_offset"]

	def format_filename(filename):
		if y_index is not None: return filename.format(x=raw_x_index, y=raw_y_index)
		else: return filename.format(x=raw_x_index)
		
	def format_tmp_filename(filename):
		if y_index is not None: return filename.format(str(raw_x_index) + "_" + str(raw_y_index))
		else: return filename.format(raw_x_index)
	
	texture_filename = os.path.join(raw_data_directory, format_filename(raw_arrangement["texture_filename_format"]))
	depth_filename = os.path.join(raw_data_directory, format_filename(raw_arrangement["depth_filename_format"]))

	disparity_yuv_filename = os.path.join(output_directory, format_filename(arrangement["depth_filename_format"]))
	texture_yuv_filename = os.path.join(output_directory, format_filename(arrangement["texture_filename_format"]))


	if mode == 'up':
		reprojected_depth_filename = os.path.join(output_directory, format_tmp_filename("reprojected_depth_{}.png"))	
		mask_filename = os.path.join(output_directory, format_tmp_filename("mask_{}.png"))

		call_tool("kinect/depth_reprojection", [
			depth_filename,
			reprojected_depth_filename,
			mask_filename,
			intrinsics_filename,
			"map"
		])

		call_tool("kinect/vsrs_disparity", [
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])

		if verbose: print "converting texture to yuv"
		png2yuv(texture_filename, texture_yuv_filename)

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(mask_filename)

	elif mode == 'down':
		reprojected_depth_filename = os.path.join(output_directory, format_tmp_filename("reprojected_depth_{}.png"))
		reprojected_texture_filename = os.path.join(output_directory, format_tmp_filename("reprojected_texture_{}.png"))	
		mask_filename = os.path.join(output_directory, format_tmp_filename("mask_{}.png"))		

		call_tool("kinect/texture_reprojection", [
			depth_filename,
			texture_filename,
			reprojected_depth_filename,
			reprojected_texture_filename,
			mask_filename,
			intrinsics_filename,
			"3",
			"telea"
		])

		call_tool("kinect/vsrs_disparity", [
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])

		if verbose: print "converting reprojected texture to yuv"
		png2yuv(reprojected_texture_filename, texture_yuv_filename)

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(reprojected_texture_filename)
		os.remove(mask_filename)

	global done_count
	done_count = done_count + 1
	if done_count > 0:
		elapsed_time = time.time() - start_time
		remaining_count = total_count - done_count
		remaining_time_estimate = (elapsed_time / done_count) * remaining_count
		print "elapsed time: {}, estimated remaining time: {}".format(format_time(elapsed_time), format_time(remaining_time_estimate))



if __name__ == '__main__':
	x_index_range = range(arrangement["x_index_range"][0], arrangement["x_index_range"][1]+1)
	total_count = arrangement["x_index_range"][1] - arrangement["x_index_range"][0] + 1

	if "y_index_range" in arrangement:
		y_index_range = range(arrangement["y_index_range"][0], arrangement["y_index_range"][1]+1)
		total_count = total_count * (arrangement["y_index_range"][1] - arrangement["y_index_range"][0] + 1)
		indices = [(x, y) for y in y_index_range for x in x_index_range]	
	else:
		indices = [(x, None) for x in x_index_range]	
	
	done_count = 0
	start_time = time.time()
	
	if not parallel:
		for xy in indices: process_view(*xy)
	else:
		Parallel(n_jobs=parallel_jobs)(delayed(process_view)(*xy) for xy in indices)

