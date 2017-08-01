#!/usr/local/bin/python
import os, string

for dirpath, dirnames, filenames in os.walk("tools/"):
	category = string.split(dirpath, "/")[1]
	if category == "": continue
	
	print(category)
	for filename in filenames:
		filename = string.split(filename, ".")[0]
		print(" - " + filename)
