#!/usr/bin/python

import sys, json

def usage_fail():
	print("usage: {} intrinsics.json out_cameras.json\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 2: usage_fail()

intrinsics_filename = sys.argv[1]
output_filename = sys.argv[2]

with open(intrinsics_filename) as intrinsics_file:
	K = json.load(intrinsics_file)

camera_count = 851
x_increment = 1.0

x = 0.0

output_array = []
for i in range(camera_count):
	Rt = [
		[1.0, 0.0, 0.0, -x],
		[0.0, 1.0, 0.0, 0.0],
		[0.0, 0.0, 1.0, 0.0],
		[0.0, 0.0, 0.0, 1.0]
	];
	x = x + x_increment

	output_camera = {
		'name' : "camera{}".format(i + 1),
		'Rt' : Rt,
		'K' : K
	};
	output_array.append(output_camera);


output = open(output_filename, 'w')
print >>output, json.dumps(output_array)
