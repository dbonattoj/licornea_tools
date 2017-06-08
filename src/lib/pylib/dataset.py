from config import *
import os, json, math

class DatasetView:
	dataset = None
	x = 0
	y = 0
	group = None
		
	def __init__(self, dataset_, x_, y_, group=None):
		self.dataset = dataset_
		self.x = x_
		self.y = y_
		self.group = group

	def local_parameters(self):
		if self.group is None: return self.dataset.parameters
		else: return self.dataset.parameters[self.group]

	def local_filename_x(self):
		par = self.local_parameters()
		x = self.x
		if "filename_x_index_factor" in par: x = math.trunc(x * par["filename_x_index_factor"])
		if "filename_x_index_offset" in par: x = x + par["filename_x_index_offset"]
		return x

	def local_filename_y(self):
		par = self.local_parameters()
		y = self.y
		if "filename_y_index_factor" in par: y = math.trunc(y * par["filename_y_index_factor"])
		if "filename_y_index_offset" in par: y = y + par["filename_y_index_offset"]
		return y
		
	def format_filename(self, tpl):
		if self.dataset.is_2d(): relpath = tpl.format(x=self.local_filename_x(), y=self.local_filename_y())
		else: relpath = tpl.format(x=self.local_filename_x())
		return self.dataset.filepath(relpath)
		
	def format_name(self, tpl):
		if self.dataset.is_2d(): return tpl.format(x=self.x, y=self.y)
		else: return tpl.format(x=self.x)
			
	def camera_name(self):
		return self.format_name(self.dataset.parameters["camera_name_format"])

	def local_parameter(self, par, default=None):
		pars = self.local_parameters()
		if par in pars: return pars[par]
		else: return default
	
	def local_filename(self, par, default=None):
		tpl = self.local_parameter(par, None)
		if tpl is not None: return self.format_filename(tpl)
		elif default is not None: return default
		else: raise Exception("no filename " + par + " in local parameters")

	def image_filename(self, default=None):
		return self.local_filename("image_filename_format", default)
	def depth_filename(self, default=None):
		return self.local_filename("depth_filename_format", default)
	def mask_filename(self, default=None):
		return self.local_filename("mask_filename_format", default)
	
	def group_view(self, grp):
		if grp is None or grp == "": return DatasetView(self.dataset, self.x, self.y, None)
		elif grp in self.dataset.parameters: return DatasetView(self.dataset, self.x, self.y, grp)
		else: raise Exception("no {} group".format(grp))


class DatasetGroup:
	dataset = None
	group = None
	
	def __init__(self, dataset_, group):
		self.dataset = dataset_
		self.group = group
	
	def parameters(self):
		if self.group == "": return self.dataset.parameters
		else: return self.dataset.parameters[self.group]
	
	def view(self, x, y=None):
		return self.dataset.view(x, y).group_view(self.group)


class Dataset:
	parameters = None
	dirname = None
	x_index_range = None
	y_index_range = None
	
	def __init__(self, parameters_filename):
		self.dirname = os.path.dirname(parameters_filename)
		with open(parameters_filename) as f:
			self.parameters = json.load(f)
		
		self.x_index_range = self.parameters["x_index_range"]
		if "y_index_range" in self.parameters:
			self.y_index_range = self.parameters["y_index_range"]		

	def group_parameters(self, grp):
		if grp is None or grp == "": return self.parameters
		elif grp in self.parameters: return self.parameters[grp]
		else: raise Exception("no {} group".format(grp))

	def filepath(self, relpath):
		return os.path.join(self.dirname, relpath)		

	def cameras_filename(self):
		return self.filepath(self.parameters["cameras_filename"])

	def is_2d(self):
		return (self.y_index_range is not None)
	def is_1d(self):
		return not self.is_2d()

	def x_min(self):
		return int(self.x_index_range[0])
	def x_max(self):
		return int(self.x_index_range[1])
	def x_step(self):
		if len(self.x_index_range) == 3: return self.x_index_range[2]
		else: return 1
	def x_valid(self, x):
		return (x >= self.x_min()) and (x <= self.x_max()) and ((x - self.x_min()) % self.x_step() == 0)
	def x_count(self):
		return (self.x_max() - self.x_min() + self.x_step()) // self.x_step()		
	def x_indices(self):
		x = self.x_min()
		while x <= self.x_max():
			yield x
			x = x + self.x_step()
	def x_count(self):
		return (self.x_max() - self.x_min() + 1) // self.x_step()
	def x_mid(self):
		return self.x_min() + (((self.x_max() - self.x_min()) // (2 * self.x_step())) * self.x_step());
	
	def y_min(self):
		if self.is_2d(): return int(self.y_index_range[0])
		else: return 0
	def y_max(self):
		if self.is_2d(): return int(self.y_index_range[1])
		else: return 0
	def y_step(self):
		if self.is_2d() and len(self.y_index_range) == 3: return self.y_index_range[2]
		else: return 1
	def y_valid(self, y):
		return (y >= self.y_min()) and (y <= self.y_max()) and ((y - self.y_min()) % self.y_step() == 0)
	def y_count(self):
		return (self.y_max() - self.y_min() + self.y_step()) // self.y_step()		
	def y_indices(self):
		y = self.y_min()
		while y <= self.y_max():
			yield y
			y = y + self.y_step()
	def y_count(self):
		if self.is_2d(): return (self.x_max() - self.x_min() + 1) // self.x_step()
		else: return 1
	def y_mid(self):
		if self.is_2d(): return self.y_min() + (((self.y_max() - self.y_min()) // (2 * self.y_step())) * self.y_step());
		else: return 0
	
	def valid(self, x, y=None):
		if self.is_2d(): return self.x_valid(x) && self.y_valid(y)
		else: return self.x_valid(x)
	
	def indices(self):
		if self.is_2d():
			for y in self.y_indices(): for x in self.x_indices(): yield (x, y)
		else:
			for x in self.x_indices(): yield (x, None)

	def view(self, x, y=None):
		if y is None:
			if self.is_2d(): raise Exception("must specify y view index, for 2d dataset")
			y = 0
		if not self.x_valid(x): raise Exception("x view index out of range")
		if not self.y_valid(y): raise Exception("y view index out of range")
		return DatasetView(self, x, y)


def encode_view_index(x, y=None):
	if y is None: return "{}".format(x)
	else: return "{},{}".format(x, y)


def decode_view_index(string):
	indices = string.split(',')
	if len(indices) == 2: return (indices[0], indices[1])
	else: return (indices[0], None)
	
