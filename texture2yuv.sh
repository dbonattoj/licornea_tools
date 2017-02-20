#!/bin/sh
echo converting $1 to $2
ffmpeg -i $1 -pix_fmt yuv420p $2 > /dev/null 2>&1


