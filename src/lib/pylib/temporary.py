from config import *
import sys, os, tempfile, threading, json, atexit

temporary_lock = threading.Lock()
temporary_dir = None
temporary_counter = 0

class temporary_file(object):
	filename = None
	extension = None
	
	def __init__(self, ext):
		self.filename = None
		self.extension = ext

	def __enter__(self):
		global temporary_lock
		global temporary_dir
		global temporary_counter
		global temporary_refcount
		temporary_lock.acquire()
		try:
			if temporary_dir is None:
				temporary_dir = tempfile.mkdtemp()
			self.filename = "{}/tmp_{}.{}".format(temporary_dir, temporary_counter, self.extension);
			temporary_counter = temporary_counter + 1
		finally:
			temporary_lock.release()
		
		with open(self.filename, 'w') as f:
			self.write_file(f)
			
		return self
	
	def __exit__(self, exc_type, exc_value, traceback):
		global temporary_lock
		global temporary_dir
		temporary_lock.acquire()
		try:
			os.unlink(self.filename)
		finally:
			temporary_lock.release()


	def write_file(self, f):
		pass


class temporary_in_json(temporary_file):
	json = None
	
	def __init__(self, j):
		super(temporary_in_json, self).__init__("json")
		self.json = j
	
	def write_file(self, f):
		json.dump(self.json, f, indent=None)


class temporary_out_json(temporary_file):
	def __init__(self):
		super(temporary_out_json, self).__init__("json")
	
	def json(self):
		with open(self.filename, 'r') as f:
			return json.load(f)


def remove_tmp_dir():
	global temporary_dir
	try:
		if temporary_dir is not None:
			os.rmdir(temporary_dir)
	except(OSError):
		pass

atexit.register(remove_tmp_dir)
