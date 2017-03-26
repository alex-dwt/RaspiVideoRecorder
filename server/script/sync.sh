#!/usr/bin/env bash

PATH_IN=/RecordedData/Tmpfs
PATH_OUT=/RecordedData/Images

cd "$PATH_IN"

while true
do
    sleep 5
    rsync -ru . "$PATH_OUT"
    count=$(ls -d */ 2>/dev/null | wc -l)
    if [ $count -gt 1 ] ; then
        rm -rf "$(ls -d */ | head -1)"
    fi;
done
