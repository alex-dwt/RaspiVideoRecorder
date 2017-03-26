#! /bin/bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
#
#DEV_ID=/dev/video2
#IMAGES_DIR=/tmp/images
#MJPG_STREAMER=/mjpg-streamer/mjpg_streamer
#FILE_DURATION=60 # seconds
#
##        pkill -SIGINT recorder.sh
##       ./recorder.sh 2>/dev/null
#
#streamerPid=-1
#
#function killStreamer {
#    kill "$streamerPid"
#    wait "$streamerPid"
#
#    if [ "$1" -eq  "1" ]; then
#        exit
#    fi
#}
#trap "killStreamer 1" INT
#
#while true
#do
#    ##############
#    # Create dir
#    ###########
#    timestamp=$(date +%s)
#    while true
#    do
#        dir="$IMAGES_DIR/$timestamp"
#        if [ -d "$dir" ]; then
#            timestamp=$((timestamp+1))
#       else
#            mkdir "$dir"
#            if [ ! -d "$dir" ]; then
#                echo "Can't create directory '$dir'"
#                exit 1
#            fi
#            break
#        fi
#    done
#
#    ##############
#    # Record images
#    ###########
#    "$MJPG_STREAMER" -i "input_uvc.so -d $DEV_ID -q 100 -r 1920x1080" -o "output_file.so -d 0 -f $dir" &
#    streamerPid=$!
#
#    timer=0
#    while true
#    do
#        if [ "$timer" -gt  "$FILE_DURATION" ]; then
#            break
#        fi
#        sleep 1
#        timer=$((timer+1))
#    done
#
#    killStreamer 0
#done