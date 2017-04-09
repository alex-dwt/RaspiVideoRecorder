#!/usr/bin/env bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

IMAGES_DIR=/RecordedData/Images

##############
# Clear old files (do not touch files with prefix "_")
###########
cd "$IMAGES_DIR"
totalCount=$(ls | grep -v ^_ | wc -l)
countToRemove=$((totalCount - $MAX_FILES_TO_KEEP))
if [ "$countToRemove" -gt  "0" ]; then
    ls | grep -m "$countToRemove" -v ^_ | xargs -n 1 -P 4 -I {} rm -rf {}
fi