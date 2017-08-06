#!/usr/local/bin/python
from pylib.dataset import *
import os, json, math, sys, shutil


def usage_fail():
	print("usage: {} parameters.json out_dirname/\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 2: usage_fail()
parameters_filename = sys.argv[1]
out_dirname = sys.argv[2]

datas = Dataset(parameters_filename)
if not datas.is_1d(): raise Exception("dataset must be 1D")

out_frame = 0
for x in datas.x_indices():
	view = datas.view(x)
	image_filename = view.image_filename()
	if not os.path.isfile(image_filename): continue

	out_frame = out_frame + 1
	out_image_filename = os.path.join(out_dirname, "frame_{:05d}.png".format(out_frame))
	shutil.copyfile(image_filename, out_image_filename)
