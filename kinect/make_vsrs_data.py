#!/usr/local/bin/python

import sys, os, subprocess

def usage_fail():
	print("usage: {} data_directory output_directory up/down\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()


data_directory = sys.argv[1]
output_directory = sys.argv[2]
mode = sys.argv[3];
if mode != "up" and mode != "down": usage_fail()

parallel = True
parallel_jobs = 8
i_range = range(1, 851+1)
intrinsics_filename = "../data/kinect_internal_intrinsics.json"
z_near = 600.0
z_far = 1600.0

tools_directory = "."

if parallel:
	from joblib import Parallel, delayed


def png2yuv(png, yuv):
	return subprocess.call("ffmpeg -n -i {} -pix_fmt yuv420p {} > /dev/null 2>&1".format(png, yuv), shell=True)

def process_view(i):
	print "## ## ## ## ## ## VIEW {:d} ## ## ## ## ## ##".format(i)

	texture_filename = os.path.join(data_directory, "texture/Kinect_out_texture_000_483.0z_0001_{:04d}.png".format(i))
	depth_filename = os.path.join(data_directory, "depth/Kinect_out_depth_000_483.0z_0001_{:04d}.png".format(i))
	
	disparity_yuv_filename = os.path.join(output_directory, "depth{:04d}.yuv".format(i))
	texture_yuv_filename = os.path.join(output_directory, "texture{:04d}.yuv".format(i))

	if mode == 'up':
		reprojected_depth_filename = os.path.join(output_directory, "reprojected_depth_{:d}.png".format(i))	
		mask_filename = os.path.join(output_directory, "mask_{:d}.png".format(i))

		print "## {:d}: reprojecting depth".format(i)		
		result = subprocess.call([
			os.path.join(tools_directory, "kinect/depth_reprojection"),
			depth_filename,
			reprojected_depth_filename,
			mask_filename,
			intrinsics_filename,
			"map"
		])
		if result != 0: raise Exception("depth_reprojection failed")

		print "## {:d}: transforming to disparity".format(i)
		result = subprocess.call([
			os.path.join(tools_directory, "kinect/vsrs_disparity"),
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])
		if result != 0: raise Exception("vsrs_disparity failed")

		print "## {:d}: converting texture to yuv".format(i)
		result = png2yuv(texture_filename, texture_yuv_filename)
		if result != 0: raise Exception("png2yuv failed")

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(mask_filename)

	elif mode == 'down':
		reprojected_depth_filename = os.path.join(output_directory, "reprojected_depth_{:d}.png".format(i))
		reprojected_texture_filename = os.path.join(output_directory, "reprojected_texture_{:d}.png".format(i))	
		mask_filename = os.path.join(output_directory, "mask_{:d}.png".format(i))		

		print "## {:d}: reprojecting texture+depth".format(i)
		result = subprocess.call([
			os.path.join(tools_directory, "kinect/texture_reprojection"),
			depth_filename,
			texture_filename,
			reprojected_depth_filename,
			reprojected_texture_filename,
			mask_filename,
			intrinsics_filename,
			"3",
			"telea"
		])
		if result != 0: raise Exception("texture_reprojection failed")

		print "## {:d}: transforming to disparity".format(i)
		result = subprocess.call([
			os.path.join(tools_directory, "kinect/vsrs_disparity"),
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])
		if result != 0: raise Exception("vsrs_disparity failed")

		print "## {:d}: converting reprojected texture to yuv".format(i)
		result = png2yuv(reprojected_texture_filename, texture_yuv_filename)
		if result != 0: raise Exception("png2yuv failed")

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(reprojected_texture_filename)
		os.remove(mask_filename)


if __name__ == '__main__':
	if not parallel:
		for i in i_range: process_view(i)
	else:
		Parallel(n_jobs=parallel_jobs)(delayed(process_view)(i) for i in i_range)

