#!/usr/local/bin/python
from pylib.dataset import *
import sys, os, json, shutil

def usage_fail():
	print("usage: {} parameters.json x_key y_key frames out_experiments.json\n".format(sys.argv[0]))
	sys.exit(1)


def f(t):
	return fig8(t)

def fig8(t):
	x = (1 - math.cos(2*math.pi*t))/2
	y = (1 + math.sin(4*math.pi*t))/2
	return (x,y)

if len(sys.argv) <= 3: usage_fail()
parameters_filename = sys.argv[1]
x_key = int(sys.argv[2])
y_key = int(sys.argv[3])
frames = int(sys.argv[4])
out_experiments_filename = sys.argv[5]

datas = Dataset(parameters_filename)
if not datas.is_2d(): raise Exception("must have 2D camera parameters")

experiments = list()
key_views = list()


for x in datas.x_indices():
	for y in datas.y_indices():
		if ((x - datas.x_min()) % x_key == 0) and ((y - datas.y_min()) % y_key == 0):
			assert(datas.valid(x, y))
			view = datas.view(x, y)
			if not os.path.isfile(view.image_filename()): raise Exception("no image for key view {}".format(encode_view_index(x, y)))
			if not os.path.isfile(view.depth_filename()): raise Exception("no depth for key view {}".format(encode_view_index(x, y)))
			idx = (x, y)
			key_views.append(idx)
			print("key: ({},{})".format(x, y))

for frame in range(frames):
	t = frame / (frames - 1.0)

	x, y = f(t)
	
	x_ind = math.trunc(x * (datas.x_max() - datas.x_min()))
	x_ind = math.trunc(x_ind / datas.x_step()) * datas.x_step()
	x_ind = x_ind + datas.x_min()
	y_ind = math.trunc(y * (datas.y_max() - datas.y_min()))
	y_ind = math.trunc(y_ind / datas.y_step()) * datas.y_step()
	y_ind = y_ind + datas.y_min()
	
	huge = 1000
	def dist_left(key_idx):
		if key_idx[0] > x_ind: return huge
		diff_x = key_idx[0] - x_ind
		diff_y = key_idx[1] - y_ind
		return diff_x*diff_x + diff_y*diff_y

	key_views = sorted(key_views, key=dist_left)
	left_key_idx = key_views[0]

	def dist_right(key_idx):
		if key_idx[0] < x_ind: return huge
		diff_x = key_idx[0] - x_ind
		diff_y = key_idx[1] - y_ind
		return diff_x*diff_x + diff_y*diff_y
		
	key_views = sorted(key_views, key=dist_right)
	right_key_idx = key_views[0]
	if right_key_idx == left_key_idx: right_key_idx = key_views[1]
	
	experiments.append([encode_view_index(*left_key_idx), encode_view_index(x_ind, y_ind), encode_view_index(*right_key_idx)])
	assert(left_key_idx != right_key_idx)


with open(out_experiments_filename, 'w') as f:
	print >>f, json.dumps(experiments)
