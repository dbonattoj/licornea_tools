#!/bin/bash

DIR=$1
OUT_DIR=$1

FILES=`ls ${DIR}/Kinect_out_texture_*.png`

for PNG in ${FILES}; do
	YUV=`basename -s .png ${PNG}`
	YUV="${OUT_DIR}/${YUV}.yuv"
	
	./texture2yuv.sh ${PNG} ${YUV} &
done

wait

