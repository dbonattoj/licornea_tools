#!/usr/local/bin/python

import sys, os, json, tempfile, subprocess, uuid

tools_directory = "."

def main(vsrs_binary_filename, parameters_filename, left_idx, virtual_idx, right_idx, output_virtual_filename, cameras_filename = None):
	if cameras_filename is None:
		cameras_filename = os.path.join(os.path.dirname(output_virtual_filename), "cameras.txt")
	
	if not os.path.exists(cameras_filename):
		with open(parameters_filename) as f:
			parameters = json.load(f)

		cameras_json_filename = os.path.join(os.path.dirname(parameters_filename), parameters["arrangement"]["cameras_filename"])
		
		subprocess.check_call([
			os.path.join(tools_directory, "camera/export_mpeg"),
			cameras_json_filename,
			cameras_filename
		])


	output_config_filename = os.path.join(os.path.dirname(output_virtual_filename), "config{}_{}_{}.txt".format(left_idx, virtual_idx, right_idx))

	import make_vsrs_config
	make_vsrs_config.main(parameters_filename, cameras_filename, left_idx, virtual_idx, right_idx, output_virtual_filename, output_config_filename)

	print "running VSRS...  virtual={} from left={} and right={}".format(virtual_idx, left_idx, right_idx)
	try:
		output = subprocess.check_output([vsrs_binary_filename, output_config_filename])
	except subprocess.CalledProcessError as err:
		print "VSRS failed on {}. Output:\n{}".format(virtual_idx, err.output)
		raise Exception("VSRS failed")
	
	if not os.path.isfile(output_virtual_filename):
		print "VSRS failed on {}. Output:\n{}".format(virtual_idx, output)
		raise Exception("VSRS failed")
		
	os.remove(output_config_filename)


if __name__ == '__main__':
	def usage_fail():
		print("usage: {} vsrs_binary parameters.json left_idx virtual_idx right_idx output_virtual.yuv [cameras.txt]\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 6: usage_fail()
	vsrs_binary_filename = sys.argv[1]
	parameters_filename = sys.argv[2]
	left_idx = int(sys.argv[3])
	virtual_idx = int(sys.argv[4])
	right_idx = int(sys.argv[5])
	output_virtual_filename = sys.argv[6]
	cameras_filename = None
	if len(sys.argv) > 7:
		cameras_filename = sys.argv[7]
		
	main(vsrs_binary_filename, parameters_filename, left_idx, virtual_idx, right_idx, output_virtual_filename, cameras_filename)
