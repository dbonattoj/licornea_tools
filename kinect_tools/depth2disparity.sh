#!/bin/sh
DEPTH=$1
DISPARITY=$2

DEPTH_MAPPED=`basename -s .png "${DEPTH}"`
DEPTH_MAPPED="${DEPTH_MAPPED}_mapped.png"
DEPTH_MAPPED_MASK=`basename -s .png "${DEPTH}"`
DEPTH_MAPPED_MASK="${DEPTH_MAPPED}_mapped_mask.png"

build/depth_reprojection ${DEPTH} ${DEPTH_MAPPED} ${DEPTH_MAPPED_MASK} ../data/kinect_intrinsics.json
build/vsrs_disparity ${DEPTH_MAPPED} ${DISPARITY} 700 1600

rm ${DEPTH_MAPPED} ${DEPTH_MAPPED_MASK}
