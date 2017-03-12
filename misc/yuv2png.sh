#/bin/sh
if [ "$#" -ne 3 ]; then
	echo "usage: yuv2png in.yuv out.png 1920x1080" >&2
	exit 1
fi
ffmpeg -y -f rawvideo -vcodec rawvideo -s $3 -pix_fmt yuv420p -i $1 -frames 1 $2 > /dev/null 2>&1

