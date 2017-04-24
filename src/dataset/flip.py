#!/usr/local/bin/python
from pylib import *
import string, json, sys

class FormatPlaceholder:
    def __init__(self, key):
        self.key = key
    def __format__(self, spec):
		key = self.key
		if key == "x":
			key = "y"
		elif key == "y":
			key = "x"
		result = key
		if spec:
			result += ":" + spec
		return "{" + result + "}"
 
class FormatDict(dict):
    def __missing__(self, key):
        return FormatPlaceholder(key)

def swap_name_template_xy(tpl):
	formatter = string.Formatter()
	mapping = FormatDict()
	return formatter.vformat(tpl, (), mapping)
	
filename_formats = [
	"image_filename_format",
	"depth_filename_format",
	"mask_filename_format"
]
	
def process_clause(par):
	swap_keys("filename_x_index_factor", "filename_y_index_factor", par)
	swap_keys("filename_x_index_offset", "filename_y_index_offset", par)
	for tpl in filename_formats:
		if tpl in par: par[tpl] = swap_name_template_xy(par[tpl])
		
		
def swap_keys(key1, key2, par):
	val1 = None
	val2 = None

	if key1 in par:
		val1 = par[key1]
		del par[key1]

	if key2 in par:
		val2 = par[key2]
		del par[key2]

	if val1 is not None:
		par[key2] = val1
	if val2 is not None:
		par[key1] = val2
		
	
	
if __name__ == '__main__':
	def usage_fail():
		print("usage: {} in_parameters.json out_parameters.json\n".format(sys.argv[0]))
		sys.exit(1)

	if len(sys.argv) <= 2: usage_fail()
	in_parameters_filename = sys.argv[1]
	out_parameters_filename = sys.argv[2]
	
	with open(in_parameters_filename) as f:
		new_parameters = json.load(f)

	swap_keys("x_index_range", "y_index_range", new_parameters)
	
	if "camera_name_format" in new_parameters:
		new_parameters["camera_name_format"] = swap_name_template_xy(new_parameters["camera_name_format"])
	
	for tpl in filename_formats:
		if tpl in new_parameters: new_parameters[tpl] = swap_name_template_xy(new_parameters[tpl])

	if "vsrs" in new_parameters:
		process_clause(new_parameters["vsrs"])
	if "kinect_raw" in new_parameters:
		process_clause(new_parameters["kinect_raw"])
	
	with open(out_parameters_filename, 'w') as f:
		print >>f, json.dumps(new_parameters, indent=4, sort_keys=True)
