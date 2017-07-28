#!/usr/local/bin/python
from pylib import *
import sys, os, json, shutil

def usage_fail():
	print("usage: {} parameters.json step out_experiments.json [limit]\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()
parameters_filename = sys.argv[1]
step = int(sys.argv[2])
out_experiments_filename = sys.argv[3]
limit = None
if len(sys.argv) > 4: limit = int(sys.argv[4])

datas = Dataset(parameters_filename)
if datas.is_2d(): raise Exception("2D camera parameters not supported")

experiments = list()

min_idx = datas.x_min()
max_idx = datas.x_max() + 1
x_index_step = datas.x_step()
if limit is not None: max_idx = min(max_idx, limit)

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
