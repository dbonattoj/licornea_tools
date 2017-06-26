# misc/touch

Create empty file and subdirectories.

## Usage

    misc/touch filename
    
Creates empty file with relative path `filename`. If necessary also creates subdirectories. For example if `filename` is `a/b/file.txt`, creates the directories `a` and `a/b` when they don't yet exist.

This is done automatically by all other C++ programs that take output filename arguments. This program is there to test if it works correctly.