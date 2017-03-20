#!/usr/local/bin/python

import sys, os, json

def usage_fail():
	print("usage: {} parameters.json step out_experiments.json [limit]\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()
parameters_filename = sys.argv[1]
step = int(sys.argv[2])
out_experiments_filename = sys.argv[3]
limit = None
if len(sys.argv) > 4: limit = int(sys.argv[4])

experiments = list()

with open(parameters_filename) as f:
	parameters = json.load(f)
arrangement = parameters["arrangement"]

if "y_index_range" in arrangement:
	raise Exception("2D camera parameters not supported")

camera_range = arrangement["x_index_range"]
min_idx = camera_range[0]
max_idx = camera_range[1]+1
if limit is not None: max_idx = min(max_idx, limit)

x_index_step = camera_range[2] if len(camera_range) == 3 else 1
if step % x_index_step != 0: raise Exception("step must be multiple of x_range step in parameters")

for index in range(min_idx, max_idx, x_index_step):
	left_index = min_idx + ((index - min_idx) // step) * step
	right_index = left_index + step
	if right_index > max_idx: break
	assert((left_index - min_idx) % x_index_step == 0)
	assert((index - min_idx) % x_index_step == 0)
	assert((right_index - min_idx) % x_index_step == 0)
	experiments.append([left_index, index, right_index])

with open(out_experiments_filename, 'w') as f:
	print >>f, json.dumps(experiments)

