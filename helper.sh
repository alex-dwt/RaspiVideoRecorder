#! /bin/bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

MACHINE_ARCH=$(uname -m | cut -c1-3 | tr '[:lower:]' '[:upper:]')
WORK_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR=$WORK_DIR/build
MJPG_STREAMER_BIN=$BUILD_DIR/mjpg_streamer

NEW_LINE=$'\n'

###################################
######### Program Entry Point #######
#################################
function main {
    if [ "$MACHINE_ARCH" != "ARM" ]; then
        echo "This program can work only at ARM machine. Abort!"
        exit 1
    fi

    case "$1" in
        "build")
            build
            ;;
        "start")
            if [[ $2 =~ ^-?[0-9]+$ ]]; then
                start $2
            else
                echo 'Specify the port for listening please!'
                exit 1
            fi
            ;;
        *)
        echo "Wrong command. Available commands are:$NEW_LINE \
- build$NEW_LINE \
- start X, where 'X' is port which the service should listen to"
        exit 1
        ;;
    esac
}

###########################
#### Create docker image  ###
#########################
function build {
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir $BUILD_DIR
        if [ $? -ne 0 ]
        then
          echo 'Can not create build directory. Abort!'
          exit 1
        fi
    fi

    if [ ! -f $MJPG_STREAMER_BIN ]; then
        echo 'Started compiling mjpg-streamer...'

        docker run --rm -it \
            -v /opt:/opt:ro \
            -v $BUILD_DIR:/mjpg-streamer-compiled \
            sdhibit/rpi-raspbian /bin/bash -c "apt-get update && \
            apt-get install -y cmake git libjpeg8-dev build-essential && \
            git clone https://github.com/jacksonliam/mjpg-streamer.git && \
            cd /mjpg-streamer/mjpg-streamer-experimental && \
            make && \
            chmod 666 *.so mjpg_streamer && \
            cp *.so mjpg_streamer /mjpg-streamer-compiled/"

        if [ $? -ne 0 ]
        then
          echo 'Can not compile mjpg-streamer. Abort!'
          exit 1
        fi

        echo 'Mjpg-streamer successfully compiled!'
    fi

    echo 'Started creating image...'
    docker build -t alex_dwt/raspi-video-recorder $WORK_DIR
    echo 'Done!'
}

###########################
#### Start docker container ###
#########################
function start {
    if [ "$(docker images | egrep -c 'alex_dwt/raspi-video-recorder')" -eq 0 ]; then
        echo "Can not find docker image. You should run 'build' command at first!"
        exit 1
    fi

    docker rm -f alex-dwt-raspi-video-recorder >/dev/null 2>&1

    docker run -d \
        -p $1:80 \
        --device /dev/vchiq:/dev/vchiq \
        -v /opt/vc:/opt/vc:ro \
        -v /tmp/images:/tmp/images \
        $(find /dev/ 2>/dev/null | egrep "/dev/video*" | xargs -I {} printf "--device={}:{} ") \
        --name alex-dwt-raspi-video-recorder \
        alex_dwt/raspi-video-recorder >/dev/null 2>&1

    if [ $? -ne 0 ]
    then
      echo 'Fail!'
    else
      echo 'Success!'
    fi
}

# execute
main "$@"