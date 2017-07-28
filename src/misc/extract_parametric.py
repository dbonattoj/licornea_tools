#!/usr/local/bin/python
from pylib.dataset import *
import os, json, math, sys, shutil

def f(t):
	return fig8(t)

def fig8(t):
	x = (1 - math.cos(2*math.pi*t))/2
	y = (1 + math.sin(4*math.pi*t))/2
	return (x,y)

def spiral(t, n = 4):
	x = (1 + t*math.sin(n*t*math.pi))/2 + (1/math.exp(n))
	y = (1 + t*math.cos(n*t*math.pi))/2 - (1/math.exp(n))
	return (x,y)
	
def circle(t):
	x = (1 + math.sin(t*2*math.pi))/2
	y = (1 + math.cos(t*2*math.pi))/2
	return (x,y)


def usage_fail():
	print("usage: {} parameters.json out_dirname/ frames\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 3: usage_fail()
parameters_filename = sys.argv[1]
out_dirname = sys.argv[2]
frames = int(sys.argv[3])

datas = Dataset(parameters_filename)
if not datas.is_2d(): raise Exception("dataset must be 2D")

images = []

print("collecting images")
for frame in range(frames):
	t = frame / (frames - 1.0)

	x, y = f(t)
	
	x_ind = math.trunc(x * (datas.x_max() - datas.x_min()))
	x_ind = math.trunc(x_ind / datas.x_step()) * datas.x_step()
	x_ind = x_ind + datas.x_min()
	y_ind = math.trunc(y * (datas.y_max() - datas.y_min()))
	y_ind = math.trunc(y_ind / datas.y_step()) * datas.y_step()
	y_ind = y_ind + datas.y_min()
	
	view = datas.view(x_ind, y_ind)
	
	image_filename = view.image_filename()
	if not os.path.isfile(image_filename): continue
	
	images.append(image_filename)

print("copying images")
out_frame = 0
for image_filename in images:
	out_frame = out_frame + 1
	out_image_filename = os.path.join(out_dirname, "frame_{:05d}.png".format(out_frame))
	shutil.copyfile(image_filename, out_image_filename)
	print("{} of {}".format(out_frame+1,len(images)))
