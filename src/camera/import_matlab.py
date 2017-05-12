#!/usr/bin/python

import sys, re, json

def usage_fail():
	print("usage: {} cameras.txt cameras.json\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 2: usage_fail()


txt_filename = sys.argv[1]
output_filename = sys.argv[2]

with open(txt_filename, 'r') as f:
	data = f.read()

numbers = re.findall(r'(-?\d*\.?\d+(e-?\d+)?)', data)

cam_count = len(numbers) // (3*3 + 3*4 + 2)
print "there are {} cameras".format(cam_count)

output_array = []
for cam_index in range(cam_count):
	i = 1
	def v():
		global i
		val = float(numbers[(3*3 + 3*4 + 2)*cam_index + i][0])
		i = i + 1
		return val
		
	K = [
		[v(), v(), v()],
		[v(), v(), v()],
		[v(), v(), v()]
	]
	i = i + 1
	Rt = [
		[v(), v(), v(), v()],
		[v(), v(), v(), v()],
		[v(), v(), v(), v()],
		[0.0, 0.0, 0.0, 1.0]
	]

	
	output_camera = {
		'name' : "camera{}".format(cam_index + 1),
		'Rt' : Rt,
		'K' : K
	};
	output_array.append(output_camera);

output = open(output_filename, 'w')
print >>output, json.dumps(output_array)
