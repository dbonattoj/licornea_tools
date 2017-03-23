#!/usr/local/bin/python

import sys, os, zipfile, subprocess, threading, time

parallel = False
parallel_jobs = 2

simulate = False

if parallel:
	from joblib import Parallel, delayed

total_count = None
start_time = None
done_count = None
done_count_lock = threading.Lock()

# band 1
x_indices = range(1, 849+1, 2)
y_indices = range(201, 301+1, 2)
y_count = 51


def format_time(seconds):
	m, s = divmod(seconds, 60)
	h, m = divmod(m, 60)
	if h > 0: return "{}h {}min {}s".format(int(h), int(m), int(s))
	elif m > 0: return "{}min {}s".format(int(m), int(s))
	else: return "{}s".format(int(s))


def process_line(y):
	zip_filename = "disparity_{:04d}.zip".format(y)
	if os.path.isfile(zip_filename): return

	zip_yuv_filenames = list()
	for x in x_indices:
		yuv_filename = "files/cam_{:04d}{:04d}.yuv".format(y, x)
		zip_yuv_filenames.append(yuv_filename)
		assert(os.path.isfile(yuv_filename))

	if simulate:
		print "{} would contain:".format(zip_filename)
		for yuv_filename in zip_yuv_filenames:
			print "- {}".format(yuv_filename) 
	else:
		print "zipping {}".format(zip_filename)
		zip_file = zipfile.ZipFile(zip_filename, "w")
		for yuv_filename in zip_yuv_filenames:
			zip_file.write(yuv_filename, arcname=os.path.basename(yuv_filename), compress_type=zipfile.ZIP_DEFLATED)

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
	done_count = 0
	start_time = time.time()
	total_count = y_count

	if not parallel:
		for y in y_indices: process_line(y)
	else:
		Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(process_line)(y) for y in y_indices)

