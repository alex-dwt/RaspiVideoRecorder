#!/usr/bin/env bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

set -e

# auto cleaner
service cron start
echo "*/2 * * * * /bin/bash -c 'export MAX_FILES_TO_KEEP=$MAX_FILES_TO_KEEP ; /recorder/script/cleaner.sh'" | crontab -

# recorder
ldconfig
v4l2-ctl -d "$VIDEO_DEV" -v width=1920,height=1080,pixelformat=MJPG

# sync from Ram to Hdd
/recorder/script/sync.sh &

npm start