#!/usr/local/bin/python
from pylib.dataset import *
from pylib.batch import *
from pylib.temporary import *
from pylib.utility import *
from pylib.config import *
import os, tempfile, uuid
import run_vsrs

if __name__ == '__main__':
	def usage_fail():
		print("usage: {} vsrs_binary parameters.json in_experiments.json out_results.txt\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 4: usage_fail()
	vsrs_binary_filename = sys.argv[1]
	parameters_filename = sys.argv[2]
	in_experiments_filename = sys.argv[3]
	out_results_filename = sys.argv[4]

	datas = Dataset(parameters_filename)

	# create the VSRS cameras file, before running VSRS
	
	cameras_filename = os.path.join(os.path.dirname(out_results_filename), "vsrs_cameras.txt")
	call_tool("camera/export_mpeg", [
		datas.cameras_filename(),
		cameras_filename
	]);

	# prepare experiments list with number
	with open(in_experiments_filename) as f:
		experiments = json.load(f)
		
	experiments_indexed = []
	n = 0
	for exp in experiments:
		experiments_indexed.append([n, exp[0], exp[1], exp[2]])
		n = n + 1
	
	psnr_results = [None] * n
	
	def run(exp_index, left_cam, this_cam, right_cam):
		this_view = datas.view(*decode_view_index(this_cam))
		real_filename = this_view.image_filename()
		
		if not os.path.isfile(real_filename): return
		
		try:
			with temporary_file("png") as tmp_virtual_filename:
				virtual_filename = tmp_virtual_filename.filename
				run_vsrs.main(vsrs_binary_filename, datas, decode_view_index(left_cam), decode_view_index(this_cam), decode_view_index(right_cam), virtual_filename, cameras_filename)
				psnr = call_tool_collect_output("misc/psnr", { real_filename, virtual_filename })
				psnr = float(psnr)
			
			psnr_results[exp_index] = psnr

		except:
			print "{} failed".format(exp_index)
	
	# run vsrs for each experiment, and evaluate psnr
	batch_process(run, experiments_indexed)
	
	# save results
	with open(out_results_filename, 'w') as f:
		print >>f, "i psnr left_x left_y virtual_x virtual_y right_x right_y"
		for i in range(n):
			exp_index, left_cam, virtual_cam, right_cam = experiments_indexed[i]
			psnr = psnr_results[i]
			left_x, left_y = decode_view_index(left_cam)
			virtual_x, virtual_y = decode_view_index(virtual_cam)
			right_x, right_y = decode_view_index(right_cam)
			if left_y is None: left_y = 0
			if virtual_y is None: virtual_y = 0
			if right_y is None: right_y = 0
			print >>f, "{:d} {:f} {:d} {:d} {:d} {:d} {:d} {:d}".format(exp_index, psnr, int(left_x), int(left_y), int(virtual_x), int(virtual_y), int(right_x), int(right_y))
	
	print "done."
