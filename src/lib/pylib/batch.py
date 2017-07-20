from .config import *
from .progress import *
import sys, os, json, math, threading

if parallel:
	from joblib import Parallel, delayed

def batch_process(function, argtup_gen):
	argtup_list = [argtup for argtup in argtup_gen]
	total_count = len(argtup_list)
	progress = Progress(total_count)
	def step(argtup):
		result = function(*argtup)
		progress.step()
		return result
	
	if not parallel:
		results = [step(argtup) for argtup in argtup_list]
	else:
		results = Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(step, check_pickle=False)(argtup) for argtup in argtup_list)
		# need threading backend because Progress uses shared variable and mutex

	print("finished {}, elapsed time {}".format(total_count, format_time(progress.elapsed_time())))

	return results


def small_batch_process(function, argtup_gen):
	argtup_list = [argtup for argtup in argtup_gen]
	total_count = len(argtup_list)
	def step(argtup):
		result = function(*argtup)
		sys.stdout.write(".")
		sys.stdout.flush()
		return result
	
	if not parallel:
		results = [step(argtup) for argtup in argtup_list]
	else:
		results = Parallel(n_jobs=parallel_jobs, backend="threading")(delayed(step, check_pickle=False)(argtup) for argtup in argtup_list)

	print("")

	return results
