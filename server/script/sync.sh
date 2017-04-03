#!/usr/bin/env bash

PATH_IN=/RecordedData/Tmpfs
PATH_OUT=/RecordedData/Images
RECORDER_BIN=v4l2-ctl

cd "$PATH_IN"

while true
do
    sleep 3
    rsync --remove-source-files -ru . "$PATH_OUT"
    pgrep "$RECORDER_BIN" > /dev/null 2>&1
    if [ $? -eq 0 ] ; then
        # if recorder is working - remove all except for the last
        rm -rf $(ls -r -d */ 2>/dev/null | tail -n +2)
    else
        #otherwise, remove all
        rm -rf *
    fi;
done
