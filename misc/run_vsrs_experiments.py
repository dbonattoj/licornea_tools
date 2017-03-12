#!/usr/local/bin/python

import sys, os, json, subprocess
import run_vsrs

tools_directory = "."
parallel = True
parallel_jobs = 8

if parallel:
	from joblib import Parallel, delayed

def yuv2png(yuv, png, size):
	subprocess.call("ffmpeg -y -f rawvideo -vcodec rawvideo -s {} -pix_fmt yuv420p -i {} -frames 1 {} > /dev/null 2>&1".format(size, yuv, png), shell=True)


def run(experiment, out_virtual_dirname):
	left_cam, this_cam, right_cam = experiment

	virtual_yuv_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{}.yuv".format(this_cam))
	virtual_rgb_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{}.png".format(this_cam))

	try:
		run_vsrs.main(vsrs_binary_filename, parameters_filename, left_cam, this_cam, right_cam, virtual_yuv_filename)

		size_string = "{}x{}".format(parameters["texture"]["width"], parameters["texture"]["height"])
		yuv2png(virtual_yuv_filename, virtual_rgb_filename, size_string)
		os.remove(virtual_yuv_filename)
	except:
		print "{} failed".format(this_cam)

if __name__ == '__main__':
	def usage_fail():
		print("usage: {} vsrs_binary parameters.json in_experiments.json out_virtual/\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 4: usage_fail()
	vsrs_binary_filename = sys.argv[1]
	parameters_filename = sys.argv[2]
	in_experiments_filename = sys.argv[3]
	out_virtual_dirname = sys.argv[4]

	with open(parameters_filename) as f:
		parameters = json.load(f)

	with open(in_experiments_filename) as f:
		experiments = json.load(f)
	
	if not parallel:
		results = [run(ex, out_virtual_dirname) for ex in experiments]
	else:
		results = Parallel(n_jobs=parallel_jobs)(delayed(run)(ex, out_virtual_dirname) for ex in experiments)
