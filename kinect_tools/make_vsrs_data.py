#!/usr/bin/python

import sys, os, subprocess

def usage_fail():
	print("usage: {} data_directory output_directory up/down\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()

data_directory = sys.argv[1]
output_directory = sys.argv[2]
mode = sys.argv[3];
if mode != "up" and mode != "down": usage_fail()

tools_directory = "../build"
intrinsics_filename = "../../data/kinect_intrinsics.json"
z_near = 700.0
z_far = 1600.0

def png2yuv(png, yuv):
	subprocess.call("ffmpeg -n -i {} -pix_fmt yuv420p {} > /dev/null 2>&1".format(png, yuv), shell=True)

for i in range(1,851+1):
	print "## ## ## ## ## ## VIEW {:d} ## ## ## ## ## ##".format(i)

	texture_filename = os.path.join(data_directory, "Kinect_out_texture_000_500.0z_0001_{:04d}.png".format(i))
	depth_filename = os.path.join(data_directory, "Kinect_out_depth_000_500.0z_0001_{:04d}.png".format(i))
	
	disparity_yuv_filename = os.path.join(output_directory, "disparity_{:04d}.yuv".format(i))
	texture_yuv_filename = os.path.join(output_directory, "texture_{:04d}.yuv".format(i))

	if mode == 'up':
		reprojected_depth_filename = os.path.join(output_directory, "reprojected_depth_{:d}.png".format(i))	
		mask_filename = os.path.join(output_directory, "mask_{:d}.png".format(i))

		print "## {:d}: reprojecting depth".format(i)
		subprocess.call([
			os.path.join(tools_directory, "depth_reprojection"),
			depth_filename,
			reprojected_depth_filename,
			mask_filename,
			intrinsics_filename,
			"map"
		])

		print "## {:d}: transforming to disparity".format(i)
		subprocess.call([
			os.path.join(tools_directory, "vsrs_disparity"),
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])

		print "## {:d}: converting texture to yuv".format(i)
		png2yuv(texture_filename, texture_yuv_filename)

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(mask_filename)

	elif mode == 'down':
		reprojected_depth_filename = os.path.join(output_directory, "reprojected_depth_{:d}.png".format(i))
		reprojected_texture_filename = os.path.join(output_directory, "reprojected_texture_{:d}.png".format(i))	
		mask_filename = os.path.join(output_directory, "mask_{:d}.png".format(i))		

		print "## {:d}: reprojecting texture+depth".format(i)
		subprocess.call([
			os.path.join(tools_directory, "texture_reprojection"),
			depth_filename,
			texture_filename,
			reprojected_depth_filename,
			reprojected_texture_filename,
			mask_filename,
			intrinsics_filename,
			"5",
			"telea"
		])

		print "## {:d}: transforming to disparity".format(i)
		subprocess.call([
			os.path.join(tools_directory, "vsrs_disparity"),
			reprojected_depth_filename,
			disparity_yuv_filename,
			str(z_near),
			str(z_far),
			"8"
		])

		print "## {:d}: converting reprojected texture to yuv".format(i)
		png2yuv(reprojected_texture_filename, texture_yuv_filename)

		# remove temporary files
		os.remove(reprojected_depth_filename)
		os.remove(reprojected_texture_filename)
		os.remove(mask_filename)

