#!/usr/bin/env bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

IMAGES_DIR=/RecordedData/Images
VIDEO_DIR=/RecordedData/Video
CONVERTER_SCRIPT=/recorder/script/converter.sh

trap "quit" SIGINT SIGTERM

function quit()
{
    pkill -f "$CONVERTER_SCRIPT" 2> /dev/null
    exit 1
}

while true
do
    folder=$(find "$IMAGES_DIR" -name ready | sed 's/\/ready//' | sort | head -n 1)

    if [ -n "$folder" ]; then
        filename=$(echo "$folder" | sed -r 's/[^_]+_([0-9]+)/\1/')
        printf "@$filename\n"
        "$CONVERTER_SCRIPT" "$folder" "$VIDEO_DIR/$filename".mkv &
        wait "$!"
        if [ "$?" -eq 0 ]; then
            rm -rf "$folder"
        else
            quit
        fi;
	fi

    printf "@\n" # current processed file is empty

    sleep 5
done
