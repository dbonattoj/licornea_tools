#!/usr/local/bin/python
#TODO convert

import sys, os, subprocess, json, time, threading, math

parallel = True
parallel_jobs = 10
tools_directory = "."
intrinsics_filename = "../data/kinect_internal_intrinsics.json"
verbose = False

make_yuv_texture = True
make_yuv_disparity = False

if parallel:
	from joblib import Parallel, delayed


def usage_fail():
	print("usage: {} parameters.json densify_method [cross_x_offset]\n".format(sys.argv[0]))
	sys.exit(1)


def clamp(minvalue, value, maxvalue):
    return max(minvalue, min(value, maxvalue))


def call_tool(tool, args):
	full_args = [os.path.join(tools_directory, tool)] + args
	if verbose:
		print "calling {}".format(tool)
		subprocess.check_call(full_args)
	else:
		try:
			subprocess.check_output(full_args)
		except subprocess.CalledProcessError as err:
			print "{} failed. output:\n{}".format(tool, err.output)
			raise
			

if len(sys.argv) <= 2: usage_fail()
parameters_filename = sys.argv[1]
densify_method = sys.argv[2];
cross_x_offset = 0
if len(sys.argv) > 3: cross_x_offset = int(sys.argv[3])


with open(parameters_filename) as f:
	parameters = json.load(f)
arrangement = parameters["arrangement"]
raw_arrangement = parameters["arrangement"]["kinect_raw"]

cameras_filename = os.path.join(os.path.dirname(parameters_filename), arrangement["cameras_filename"])

z_near = parameters["depth"]["z_near"]
z_far = parameters["depth"]["z_far"]

total_count = None
start_time = None
done_count = None
done_count_lock = threading.Lock()

def format_time(seconds):
	m, s = divmod(seconds, 60)
	h, m = divmod(m, 60)
	if h > 0: return "{}h {}min {}s".format(int(h), int(m), int(s))
	elif m > 0: return "{}min {}s".format(int(m), int(s))
	else: return "{}s".format(int(s))


def png2yuv(png, yuv):
	if os.path.isfile(yuv): os.remove(yuv)
	subprocess.check_call("ffmpeg -n -i {} -pix_fmt yuv420p {} > /dev/null 2>&1".format(png, yuv), shell=True)

def process_view(x_index, y_index):	
	if verbose: print "view x={}, y={}".format(x_index, y_index)
			
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

	reprojected_depth_filename = os.path.join(os.path.dirname(parameters_filename), format_tmp_filename("reprojected_depth_{}.png"))	
	point_cloud_filename = os.path.join(os.path.dirname(parameters_filename), format_tmp_filename("point_cloud_{}.ply"))
	mask_filename = os.path.join(os.path.dirname(parameters_filename), format_tmp_filename("mask_{}.png"))

	if make_yuv_disparity and not os.path.isfile(disparity_yuv_filename):
		if x_in_index == x_out_index:
			call_tool("kinect/depth_reprojection", [
				depth_filename,
				reprojected_depth_filename,
				mask_filename,
				intrinsics_filename,
				densify_method
			])
		else:
			in_depth_filename = os.path.join(os.path.dirname(parameters_filename), format_raw_in_filename(raw_arrangement["depth_filename_format"]))
			
			call_tool("kinect/depth_point_cloud", [
				in_depth_filename,
				point_cloud_filename,
				intrinsics_filename,
				"color"
			])
			
			if y_index is not None:
				in_camera_name = arrangement["camera_name_format"].format(x=x_in_index, y=y_index)
				out_camera_name = arrangement["camera_name_format"].format(x=x_out_index, y=y_index)
			else:
				in_camera_name = arrangement["camera_name_format"].format(x=x_in_index)
				out_camera_name = arrangement["camera_name_format"].format(x=x_out_index)
			
			call_tool("kinect/point_cloud_reprojection", [
				point_cloud_filename,
				reprojected_depth_filename,
				mask_filename,
				intrinsics_filename,
				densify_method,
				cameras_filename,
				in_camera_name,
				out_camera_name
			])
	
	
		call_tool("kinect/vsrs_disparity", [
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])

		# remove temporary files
		if os.path.isfile(point_cloud_filename): os.remove(point_cloud_filename)
		os.remove(reprojected_depth_filename)
		os.remove(mask_filename)



	if make_yuv_texture and not os.path.isfile(texture_yuv_filename):
		if verbose: print "converting texture to yuv"
		png2yuv(texture_filename, texture_yuv_filename)

	global done_count_lock
	global done_count
	done_count_lock.acquire()
	try:
		done_count = done_count + 1
		if done_count > 0:
			elapsed_time = time.time() - start_time
			remaining_count = total_count - done_count
			views_per_second = done_count / elapsed_time
			remaining_time_estimate = remaining_count / views_per_second
			print "done {} of {}. elapsed time: {}, estimated remaining time: {}".format(done_count, total_count, format_time(elapsed_time), format_time(remaining_time_estimate))
	finally:
		done_count_lock.release()


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
	
	done_count = 0
	start_time = time.time()
	
	if not parallel:
		for xy in indices:
			process_view(*xy)
	else:
		Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(process_view)(*xy) for xy in indices)
		# need threading backend because of shared 'done_count' variable

	print "done."
	
