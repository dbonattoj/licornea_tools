import os

# If True, additional output is printed. Useful for debugging.
verbose = (int(os.getenv("LICORNEA_VERBOSE", "0")) != 0)

# If True, batch jobs are run in parallel.
parallel = (int(os.getenv("LICORNEA_PARALLEL", "1")) != 0)

# If parallel if True, number of parallel jobs to launch.
# Should be adjusted to get best efficiency. For jobs that access a lot of different files, should be low,
# otherwise time is lost seeking the disk drive.
parallel_jobs = int(os.getenv("LICORNEA_NUM_THREADS", "8"))


