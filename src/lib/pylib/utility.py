from config import *
import sys, os, subprocess, platform, json

def running_on_windows():
	return (platform.system() == "Windows")

def clamp(minvalue, value, maxvalue):
    return max(minvalue, min(value, maxvalue))

def tools_directory():
	return os.path.dirname(os.path.dirname(os.path.realpath(sys.argv[0])))

def call_tool(tool, args):
	if running_on_windows():
		tool = tool + ".exe"
		
	full_args = [os.path.join(tools_directory(), tool)] + args
	if verbose:
		print "calling {}".format(tool)
		subprocess.check_call(full_args)
	else:
		try:
			subprocess.check_output(full_args)
		except subprocess.CalledProcessError as err:
			print "{} failed. output:\n{}".format(tool, err.output)
			raise


def call_tool_collect_output(tool, args):
	if running_on_windows():
		tool = tool + ".exe"
	full_args = [os.path.join(tools_directory(), tool)] + args
	if verbose: print "calling {}".format(tool)
	try:
		return subprocess.check_output(full_args)
	except subprocess.CalledProcessError as err:
		print "{} failed. output:\n{}".format(tool, err.output)
		raise

def yuv2png(yuv, png, width, height):
	size = "{}x{}".format(width, height)
	if running_on_windows(): raise Exception("TODO windows ffmpeg")
	else: subprocess.call("ffmpeg -y -f rawvideo -vcodec rawvideo -s {} -pix_fmt yuv420p -i {} -frames 1 {} > /dev/null 2>&1".format(size, yuv, png), shell=True)

def png2yuv(png, yuv):
	if running_on_windows(): raise Exception("TODO windows ffmpeg")
	else: subprocess.check_call("ffmpeg -y -i {} -pix_fmt yuv420p {} > /dev/null 2>&1".format(png, yuv), shell=True)

def format_time(seconds):
	m, s = divmod(seconds, 60)
	h, m = divmod(m, 60)
	if h > 0: return "{}h {}min {}s".format(int(h), int(m), int(s))
	elif m > 0: return "{}min {}s".format(int(m), int(s))
	else: return "{}s".format(int(s))

def import_json_file(filename):
	with open(filename, 'r') as f:
		j = json.load(f)
	return j
			
def export_json_file(j, filename):
	with open(filename, 'w') as f:
		j.dump(f)
