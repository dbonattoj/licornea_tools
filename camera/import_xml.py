#!/usr/bin/python

import sys, re, json

def usage_fail():
	print("usage: {} cameras.xml cameras.json\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 2: usage_fail()


xml_filename = sys.argv[1]
output_filename = sys.argv[2]

import xml.etree.ElementTree
root = xml.etree.ElementTree.parse(xml_filename).getroot()

#root = root.find('OriginalCameras')

output_array = []

index = -1
for camera in root.findall('Camera'):
	index = index + 1
	camera_name = camera.find('name').text
	K_str = camera.find('K').text
	Rt_str = camera.find('Rt').text
	
	K = re.findall(r'-?[0-9.]+', K_str)	
	Rt = re.findall(r'-?[0-9.]+', Rt_str)

	if len(K) != 3*3 or len(Rt) != 3*4:
		print "skipping entry {} (no K or Rt matrix)".format(index)
		continue

	assert(len(K) == 3*3)
	K = [float(x) for x in K]
	assert(len(Rt) == 3*4)
	Rt = [float(x) for x in Rt]

	output_Rt = [
		[Rt[0], Rt[1], Rt[2], Rt[3]],
		[Rt[4], Rt[5], Rt[6], Rt[7]],
		[Rt[8], Rt[9], Rt[10], Rt[11]],
		[0.0, 0.0, 0.0, 1.0]
	];
	
	output_K = [
		[K[0], K[1], K[2]],
		[K[3], K[4], K[5]],
		[K[6], K[7], K[8]]
	];

	output_camera = {
		'name' : camera_name,
		'Rt' : output_Rt,
		'K' : output_K
	};
	output_array.append(output_camera);


output = open(output_filename, 'w')
print >>output, json.dumps(output_array)
