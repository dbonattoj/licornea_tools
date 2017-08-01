# Documentation

These are several programs that do single processing steps in the 3DLicorneA pipeline. Each program works independently, taking some input data and generating output data. Parameters are usually exchanged in JSON-based formats.

The programs are organized into thematic groups, in the different subdirectories `vsrs/`, `kinect/`, etc. Each `.py` and each `.cc` file is one program. Only the C++ programs operate on the content of images. Some Python programs are scripts that call other programs on a set of inputs, to do batch operations. (For example, exporting an entire dataset to VSRS format).

Building installs all the executables/scripts into the `bin/` directory, from which they should be used.

