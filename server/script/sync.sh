#!/usr/bin/env bash

PATH_IN=/tmp/v4l
PATH_OUT=/tmp/saved

mkdir "$PATH_OUT"
cd "$PATH_IN"

while true
do
    sleep 5
    rsync -ruv . "$PATH_OUT"
    count=$(ls -d */ | wc -l)
    if [ $count -gt 1 ] ; then
        rm -rf "$(ls -d */ | head -1)"
    fi;
done
