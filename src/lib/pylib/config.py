# If True, additional output is printed. Useful for debugging.
verbose = False

# If True, batch jobs are run in parallel.
parallel = True

# If parallel if True, number of parallel jobs to launch.
# Should be adjusted to get best efficiency. For jobs that access a lot of different files, should be low,
# otherwise time is lost seeking the disk drive.
parallel_jobs = 4

