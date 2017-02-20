#!/bin/bash

DIR=$1
OUT_DIR=$1

FILES=`ls ${DIR}/Kinect_out_depth_*.png`

for DEPTH in ${FILES}; do
	DISPARITY=`basename -s .png ${DEPTH}`
	DISPARITY=`echo ${DISPARITY} | sed -e 's/depth/disparity/'`
	DISPARITY="${OUT_DIR}/${DISPARITY}.yuv"
	
	./depth2disparity.sh ${DEPTH} ${DISPARITY} &
done

wait

