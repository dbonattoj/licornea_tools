#!/bin/bash

mkdir -p tmp

# band 1
for NUM in {1..101..2};
do
	NUM=`printf %04d $NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_240317/Band1_0001_0101/ULB_Row$NUM.xml tmp/$NUM.json > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band1.json
rm tmp/*.json


# band 2
for NUM in {101..201};
do
	NUM=`printf %04d $NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_240317/Band2_0101_0201/ULB_Row$NUM.xml tmp/$NUM.json > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band2.json
rm tmp/*.json


# band 3
for NUM in {201..301..2};
do
	NUM=`printf %04d $NUM`
	../bin/camera/import_xml.py ../data/170317/raw/param_240317/Band3_0201_0301/ULB_Row$NUM.xml tmp/$NUM.json > /dev/null
done
jq -s 'reduce .[] as $item ([]; . + $item)' tmp/*.json > cameras_band3.json
rm tmp/*.json


#jq -s 'reduce .[] as $item ([]; . + $item)' cameras_band3.json cameras_band2.json cameras_band1.json > cameras.json


