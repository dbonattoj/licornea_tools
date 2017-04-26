from config import *
from progress import *
import os, json, math, threading

if parallel:
	from joblib import Parallel, delayed

def batch_process(function, argtup_gen):
	argtup_list = [argtup for argtup in argtup_gen]
	total_count = len(argtup_list)
	progress = Progress(total_count)
	def step(argtup):
		function(*argtup)
		progress.step()
	
	if not parallel:
		for argtup in argtup_list: step(argtup)
	else:
		Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(step, check_pickle=False)(argtup) for argtup in argtup_list)
		# need threading backend because Progress uses shared variable and mutex

	print "finished {}, elapsed time {}".format(total_count, format_time(progress.elapsed_time()))
