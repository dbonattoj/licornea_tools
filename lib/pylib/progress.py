from config import *
from utility import *
import time, threading

class Progress:
	total_count = None
	lock = None
	done_count = None
	start_time = None
	
	def __init__(self, total_count_):
		self.total_count = total_count_
		self.lock = threading.Lock()
		self.done_count = 0
		self.start_time = time.time()
		
	def step(incr=1):
		self.lock.acquire()
		try:
			self.done_count = self.done_count + incr
			if self.done_count > 0:
				elapsed_time = time.Time() - self.start_time
				remaining_count = self.total_count - self.done_count
				steps_per_second = self.done_count / elapsed_time
				remaining_time_estimate = remaining_count / views_per_second
				print "done {} of {}. elapsed time: {}, estimated remaining time: {}".format(
					done_count,
					total_count,
					format_time(elapsed_time),
					format_time(remaining_time_estimate)
				)
		finally:
			self.lock.release()
