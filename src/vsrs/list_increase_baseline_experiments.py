#!/usr/local/bin/python
from pylib.dataset import *
import os, json, sys

def usage_fail():
	print("usage: {} parameters.json center max_baseline out_experiments.json [step]\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 4: usage_fail()
parameters_filename = sys.argv[1]
center = int(sys.argv[2])
max_baseline = int(sys.argv[3])
out_experiments_filename = sys.argv[4]
step = 1
if len(sys.argv) > 5: step = int(sys.argv[5])

datas = Dataset(parameters_filename)
if datas.is_2d(): raise Exception("2D camera parameters not supported")

experiments = list()


min_idx = datas.x_min()
max_idx = datas.x_max() + 1
x_index_step = datas.x_step()

for rad in range(1, max_baseline // 2, x_index_step*step):
	index = center
	left_index = index - rad
	right_index = index + rad
	if not(datas.x_valid(index) and datas.x_valid(left_index) and datas.x_valid(right_index)):
		continue
	experiments.append([encode_view_index(left_index), encode_view_index(index), encode_view_index(right_index)])

with open(out_experiments_filename, 'w') as f:
	print >>f, json.dumps(experiments)

