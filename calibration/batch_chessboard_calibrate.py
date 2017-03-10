#!/usr/bin/python

import sys, os, subprocess, json
from joblib import Parallel, delayed

def usage_fail():
	print("usage: {} data_directory output_directory\n".format(sys.argv[0]))
	sys.exit(1)

if len(sys.argv) <= 2: usage_fail()

index_count = 100
data_directory = sys.argv[1]
output_directory = sys.argv[2]
intrinsic_filename = "data/kinect_color_intrinsic.json"

tools_directory = "build"

def generate_extrinsic(i, initial_extrinsic, keep_cors):
	if initial_extrinsic is None:
		print "estimating extrinsic for {}".format(i)
	else:
		print "refining extrinsic for {}".format(i)

	texture_filename = os.path.join(data_directory, "Kinect_out_texture_000_483.0z_0001_{:04d}.png".format(i+1))
	cors_filename = os.path.join(output_directory, "cors_{}.json".format(i+1))
	extrinsic_filename = os.path.join(output_directory, "extrinsic_{}.json".format(i+1))

	if not os.path.isfile(cors_filename):
		find_chessboard_result = subprocess.call([
			os.path.join(tools_directory, "find_chessboard"),
			texture_filename,
			cors_filename,
			"6",
			"8",
			"27.0"
		])
		if find_chessboard_result != 0: return None	

	args = [
		os.path.join(tools_directory, "calibrate_extrinsic"),
		cors_filename,
		intrinsic_filename,
		extrinsic_filename
	]
	if initial_extrinsic is not None:
		initial_extrinsic_filename = os.path.join(output_directory, "initial_extrinsic_{}.json".format(i+1))
		with open(initial_extrinsic_filename, 'w') as initial_extrinsic_file:
			print >>initial_extrinsic_file, json.dumps(initial_extrinsic)
		args.append(initial_extrinsic_filename)
	subprocess.call(args)

	with open(extrinsic_filename) as extrinsic_file:
		extrinsic = json.load(extrinsic_file)

	if not keep_cors: os.remove(cors_filename)
	os.remove(extrinsic_filename)
	if initial_extrinsic is not None: os.remove(initial_extrinsic_filename)

	return extrinsic
	
	
def export_camera_params(extrinsics, filename):
	with open(intrinsic_filename) as intrinsic_file:
		intrinsic = json.load(intrinsic_file)

	camera_params = list()
	for i in range(index_count):
		if extrinsics[i] is None: continue
		camera_param = dict()
		camera_param["name"] = "camera_{}".format(i + 1)
		camera_param["K"] = intrinsic
		camera_param["Rt"] = extrinsics[i]
		camera_params.append(camera_param)

	with open(filename, 'w') as f:
		print >>f, json.dumps(camera_params)


def import_camera_params(filename):
	with open(filename) as f:
		camera_params = json.load(f)
	return [cam['Rt'] for cam in camera_params]


if __name__ == '__main__':
	print "Initial extrinsics estimation"
	extrinsics = [generate_extrinsic(i, None, True) for i in range(index_count)]

	camera_params_filename = os.path.join(output_directory, "params.json")
	export_camera_params(extrinsics, camera_params_filename);
		
	print "Denoising row"
	camera_params_de_filename = os.path.join(output_directory, "params_de.json")
	subprocess.call([
		os.path.join(tools_directory, "denoise_extrinsic_row"),
		camera_params_filename,
		camera_params_de_filename
	])
	
	camera_params_de = import_camera_params(camera_params_de_filename)
	
	print "Refining extrinsics estimation"
	extrinsics_refined = [generate_extrinsic(i, camera_params_de[i], False) for i in range(index_count)]

	camera_params_refined_filename = os.path.join(output_directory, "params_refined.json")
	export_camera_params(extrinsics_refined, camera_params_refined_filename)
