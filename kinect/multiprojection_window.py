#!/usr/local/bin/python

import sys, os, json

def usage_fail():
	print("usage: {} parameters.json out_multiprojection.json x_out_idx [y_out_idx]\n".format(sys.argv[0]))
	sys.exit(1)


def clamp(minvalue, value, maxvalue):
    return max(minvalue, min(value, maxvalue))

if __name__ == '__main__':
	if len(sys.argv) <= 3: usage_fail()
	parameters_filename = sys.argv[1]
	out_multiprojection_filename = sys.argv[2]
	x_out_idx = int(sys.argv[3])
	y_out_idx = None
	if len(sys.argv) > 4: y_out_idx = int(sys.argv[4])
	
	center_x_idx_off = 50
	center_y_idx_off = 0
	
	x_window_rad = 4
	y_window_rad = 0

	def point_cloud_filename(x_idx, y_idx = None):
		if y_idx is None: return "point_cloud_{}.ply".format(x_idx)
		else: return "point_cloud_{}_{}.ply".format(x_idx, y_idx)


	with open(parameters_filename) as f:
		parameters = json.load(f)
	arrangement = parameters["arrangement"]
	raw_arrangement = arrangement["kinect_raw"]

	mprojs = []

	if "y_index_range" in arrangement:
		if y_out_idx is None: raise Exception("for 2D set, y_index must be specified")
				
		x_in_min_idx = clamp(arrangement["x_index_range"][0], x_out_idx + center_x_idx_off - x_window_rad, arrangement["x_index_range"][1])
		x_in_max_idx = clamp(arrangement["x_index_range"][0], x_out_idx + center_x_idx_off + x_window_rad, arrangement["x_index_range"][1])

		y_in_min_idx = clamp(arrangement["y_index_range"][0], y_out_idx + center_y_idx_off - y_window_rad, arrangement["y_index_range"][1])
		y_in_max_idx = clamp(arrangement["y_index_range"][0], y_out_idx + center_y_idx_off + y_window_rad, arrangement["y_index_range"][1])
		
		for y_in_idx in range(y_in_min_idx, y_in_max_idx+1):
			for x_in_idx in range(x_in_min_idx, x_in_max_idx+1):
				input_point_cloud_filename = point_cloud_filename(x_in_idx, y_in_idx)	
				camera_name = arrangement["camera_name_format"].format(x=x_in_idx, y=y_in_idx)
				mproj_input = {
					"x_idx" : x_in_idx,
					"y_idx" : y_in_idx,
					"point_cloud_filename" : input_point_cloud_filename,
					"camera_name" : camera_name
				}
				mprojs.append(mproj_input)
		
	else:
		x_in_min_idx = clamp(arrangement["x_index_range"][0], x_out_idx + center_x_idx_off - x_window_rad, arrangement["x_index_range"][1])
		x_in_max_idx = clamp(arrangement["x_index_range"][0], x_out_idx + center_x_idx_off + x_window_rad, arrangement["x_index_range"][1])
		
		for x_in_idx in range(x_in_min_idx, x_in_max_idx+1):
			input_point_cloud_filename = point_cloud_filename(x_in_idx)	
			camera_name = arrangement["camera_name_format"].format(x=x_in_idx)
			mproj_input = {
				"x_idx" : x_in_idx,
				"point_cloud_filename" : input_point_cloud_filename,
				"camera_name" : camera_name
			}
			mprojs.append(mproj_input)


	with open(out_multiprojection_filename, 'w') as f:
		print >>f, json.dumps(mprojs)

