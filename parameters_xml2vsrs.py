#!/usr/bin/python

import sys, re

xml_filename = sys.argv[1]
output_filename = sys.argv[2]

output = open(output_filename, 'w')

import xml.etree.ElementTree
root = xml.etree.ElementTree.parse(xml_filename).getroot()

for camera in root.findall('Camera'):
	camera_name = camera.find('name').text
	K_str = camera.find('K').text
	Rt_str = camera.find('Rt').text
	
	K = re.findall(r'-?[0-9.]+', K_str)
	assert(len(K) == 3*3)
	Rt = re.findall(r'-?[0-9.]+', Rt_str)
	assert(len(Rt) == 3*4)


	print >>output, camera_name
	print >>output, str(K[0]), " ", str(K[1]), " ", str(K[2])
	print >>output, str(K[3]), " ", str(K[4]), " ", str(K[5])
	print >>output, str(K[6]), " ", str(K[7]), " ", str(K[8])
	print >>output, "0\n0"
	print >>output, str(Rt[0]), " ", str(Rt[1]), " ", str(Rt[2]), " ", str(Rt[3])
	print >>output, str(Rt[4]), " ", str(Rt[5]), " ", str(Rt[6]), " ", str(Rt[7])
	print >>output, str(Rt[8]), " ", str(Rt[9]), " ", str(Rt[10]), " ", str(Rt[11])
	print >>output, ""

