#!/usr/local/bin/python

import sys, os, json

def usage_fail():
	print("usage: {} parameters.json step out_experiments.json\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()
parameters_filename = sys.argv[1]
step = int(sys.argv[2])
out_experiments_filename = sys.argv[3]

experiments = list()

with open(parameters_filename) as f:
	parameters = json.load(f)

camera_range = parameters["arrangement"]["x_index_range"]
for index in range(camera_range[0], camera_range[1]+1):
	left_index = camera_range[0] + ((index - camera_range[0]) // step) * step
	right_index = left_index + step
	if right_index > camera_range[1]: break
	experiments.append([left_index, index, right_index])

with open(out_experiments_filename, 'w') as f:
	print >>f, json.dumps(experiments)

