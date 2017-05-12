#!/usr/local/bin/python
from pylib import *
import os, tempfile, uuid
import run_vsrs

datas = None
out_virtual_dirname = None
cameras_filename = None

def run(exp_index, left_cam, this_cam, right_cam):	
	virtual_yuv_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{:05d}.yuv".format(exp_index))
	virtual_rgb_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{:05d}.png".format(exp_index))

	try:
		run_vsrs.main(vsrs_binary_filename, parameters_filename, left_cam, this_cam, right_cam, virtual_yuv_filename, cameras_filename)

		yuv2png(virtual_yuv_filename, virtual_rgb_filename, datas.parameters["width"], datas.parameters["height"])
		os.remove(virtual_yuv_filename)
	except:
		print "{} failed".format(this_cam)
		raise
		

if __name__ == '__main__':
	def usage_fail():
		print("usage: {} vsrs_binary parameters.json in_experiments.json out_virtual/\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 4: usage_fail()
	vsrs_binary_filename = sys.argv[1]
	parameters_filename = sys.argv[2]
	in_experiments_filename = sys.argv[3]
	out_virtual_dirname = sys.argv[4]

	datas = Dataset(parameters_filename)

	# create the VSRS cameras file, before running VSRS
	# run_vsrs() would also create a cameras file if none exists yet
	# but: when running in parallel, 2+ of the first launched instances could start writing a cameras file,
	#      and then one of them could launch VSRS, while the another is overwriting the cameras file, and so it is incomplete for VSRS
	cameras_filename = os.path.join(out_virtual_dirname, "vsrs_cameras.txt")
	call_tool("camera/export_mpeg", [
		datas.cameras_filename(),
		cameras_filename
	]);

	with open(in_experiments_filename) as f:
		experiments = json.load(f)
		
	experiments_indexed = []
	i = 0
	for exp in experiments:
		experiments_indexed.append([i, exp[0],exp[1],exp[2]])
		i = i + 1
	
	batch_process(run, experiments_indexed)
	
	print "done."
