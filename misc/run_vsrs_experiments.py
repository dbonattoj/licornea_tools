#!/usr/local/bin/python

import sys, os, json, subprocess, time, threading
import run_vsrs

tools_directory = "."
parallel = True
parallel_jobs = 8

total_count = None
start_time = None
done_count = None
done_count_lock = threading.Lock()

if parallel:
	from joblib import Parallel, delayed


def format_time(seconds):
	m, s = divmod(seconds, 60)
	h, m = divmod(m, 60)
	if h > 0: return "{}h {}min {}s".format(int(h), int(m), int(s))
	elif m > 0: return "{}min {}s".format(int(m), int(s))
	else: return "{}s".format(int(s))

def yuv2png(yuv, png, size):
	subprocess.call("ffmpeg -y -f rawvideo -vcodec rawvideo -s {} -pix_fmt yuv420p -i {} -frames 1 {} > /dev/null 2>&1".format(size, yuv, png), shell=True)


def run(experiment, out_virtual_dirname):
	left_cam, this_cam, right_cam = experiment
	
	virtual_yuv_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{:05d}.yuv".format(this_cam))
	virtual_rgb_filename = os.path.join(os.path.dirname(out_virtual_dirname), "virtual_{:05d}.png".format(this_cam))

	try:
		run_vsrs.main(vsrs_binary_filename, parameters_filename, left_cam, this_cam, right_cam, virtual_yuv_filename)

		size_string = "{}x{}".format(parameters["texture"]["width"], parameters["texture"]["height"])
		yuv2png(virtual_yuv_filename, virtual_rgb_filename, size_string)
		os.remove(virtual_yuv_filename)
	except:
		print "{} failed".format(this_cam)
		raise
		
		
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
	
	total_count = len(experiments)
	done_count = 0
	start_time = time.time()

	if not parallel:
		for ex in experiments:
			run(ex, out_virtual_dirname)
	else:
		Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(run)(ex, out_virtual_dirname) for ex in experiments)

	print "done."
