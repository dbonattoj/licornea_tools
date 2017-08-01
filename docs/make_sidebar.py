#!/usr/local/bin/python
import os, string

tools = []
datas = []

for dirpath, dirnames, filenames in os.walk("tools/"):
	category = string.split(dirpath, "/")[1]
	if category == "": continue
	
	for filename in filenames:
		name = string.split(filename, ".")[0]
		tools.append( (category, name) )

tools = sorted(tools, key=lambda tup: tup[0] + "/" + tup[1])

sidebar_filename = "_includes/sidebar.html"
last_category = ""
with open(sidebar_filename, "w") as f:
	print >>f, "<small>"
	for category, name in tools:
		if category != last_category:
			print >>f, "<a name=\"" + category + "\"></a><strong>" + category + "</strong><br/>"
			last_category = category
		
		link = "/tools/" + category + "/" + name + ".html"
		link = "{{ '" + link + "' | relative_url }}"

		print >>f, "&nbsp;&nbsp;&nbsp;<a href=\"" + link + "\">" + name + "</a><br/>"
		
	print >>f, "</small>"
