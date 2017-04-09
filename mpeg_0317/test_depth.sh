#!/bin/bash

METHOD=mine

rm -rf tmp
mkdir tmp

# left
../bin/kinect/depth_reprojection ../data/170317/raw/483-583_step1mm_acq2/depth/500.0/Kinect_out_depth_000_500.0z_0001_0350.png tmp/de1.png tmp/m1.png ../data/kinect_internal_intrinsics.json $METHOD

../bin/kinect/vsrs_disparity tmp/de1.png tmp/disp1.yuv 600 1600

ffmpeg -i ../data/170317/raw/483-583_step1mm_acq2/texture/500.0/Kinect_out_texture_000_500.0z_0001_0350.png -pix_fmt yuv420p tmp/tex1.yuv > /dev/null 2>&1


# right
../bin/kinect/depth_reprojection ../data/170317/raw/483-583_step1mm_acq2/depth/500.0/Kinect_out_depth_000_500.0z_0001_0450.png tmp/de2.png tmp/m2.png ../data/kinect_internal_intrinsics.json $METHOD

../bin/kinect/vsrs_disparity tmp/de2.png tmp/disp2.yuv 600 1600

ffmpeg -i ../data/170317/raw/483-583_step1mm_acq2/texture/500.0/Kinect_out_texture_000_500.0z_0001_0450.png -pix_fmt yuv420p tmp/tex2.yuv > /dev/null 2>&1


# cameras
../bin/camera/import_xml.py ../data/170317/raw/param_240317/Band2_0101_0201/ULB_Row0184.xml tmp/cams.json
../bin/camera/export_mpeg tmp/cams.json tmp/cams.txt


# synthesis
../vsrs/vsrs-4.1 test_depth_conf.txt

rm -f 5.bmp 6.bmp 7.bmp

../bin/misc/yuv2png.sh tmp/out.yuv out.png 1920x1080

../bin/misc/view_depth tmp/de1.png 600 1600 depth.png

