#!/bin/bash

mkdir -p tmp

# band 1
for PARAM_NUM in {383..483..2};
do
	VIEW_NUM=$((684 - $PARAM_NUM))
	VIEW_NUM=`printf %04d $VIEW_NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_yan_220317/Band1_383_483/ULB_Line$PARAM_NUM.xml tmp/$VIEW_NUM.json > /dev/null
	../bin/camera/transform tmp/$VIEW_NUM.json replace rename cam_$VIEW_NUM{:04d} 1 2 > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band1.json
rm tmp/*.json


# band 2
for PARAM_NUM in {483..583};
do
	#VIEW_NUM=$((684 - $PARAM_NUM))
	VIEW_NUM=$(($PARAM_NUM - 382))
	VIEW_NUM=`printf %04d $VIEW_NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_yan_220317/Band2_483_583/ULB_Line$PARAM_NUM.xml tmp/$VIEW_NUM.json > /dev/null
	../bin/camera/transform tmp/$VIEW_NUM.json replace rename cam_$VIEW_NUM{:04d} 1  > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band2.json
rm tmp/*.json


# band 3
for PARAM_NUM in {583..683..2};
do
	#VIEW_NUM=$((684 - $PARAM_NUM))
	VIEW_NUM=$(($PARAM_NUM - 582))
	VIEW_NUM=`printf %04d $VIEW_NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_yan_220317/Band3_583_683/ULB_Line$PARAM_NUM.xml tmp/$VIEW_NUM.json > /dev/null
	../bin/camera/transform tmp/$VIEW_NUM.json replace rename cam_$VIEW_NUM{:04d} 1 2 > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band3.json
rm tmp/*.json


#jq -s 'reduce .[] as $item ([]; . + $item)' cameras_band3.json cameras_band2.json cameras_band1.json > cameras.json


