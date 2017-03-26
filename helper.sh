#! /bin/bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

# choose desirable camera
VIDEO_DEV=/dev/video0
# enter desirable count of files to keep
MAX_FILES_TO_KEEP=10

MACHINE_ARCH=$(uname -m | cut -c1-3 | tr '[:lower:]' '[:upper:]')
WORK_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$WORK_DIR/build"
V4L2_DEB="$BUILD_DIR/v4l2.deb"
CONTAINER_NAME=alex-dwt-raspi-video-recorder
IMAGE_NAME=alex_dwt/raspi-video-recorder

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
        mkdir "$BUILD_DIR"
        if [ $? -ne 0 ]
        then
          echo 'Can not create build directory. Abort!'
          exit 1
        fi
    fi

    if [ ! -f "$V4L2_DEB" ]; then
        echo 'Started compiling v4l2-utils...'

        docker run --rm -it \
            -v "$BUILD_DIR":/v4l2-compiled \
            sdhibit/rpi-raspbian /bin/bash -c "apt-get update \
            && apt-get install -y debhelper dh-autoreconf autotools-dev doxygen graphviz \
                libasound2-dev libtool libjpeg-dev libqt4-dev libqt4-opengl-dev libudev-dev \
                libx11-dev pkg-config udev git checkinstall \
            && git clone https://github.com/alex-dwt/v4l-utils \
            && cd v4l-utils && sed -i 's/1%{?dist}/1/g' v4l-utils.spec.in \
            && ./bootstrap.sh && ./configure --without-jpeg && make -j4 \
            && checkinstall --install=no  --fstrans=no -y \
            && cp *deb /v4l2-compiled/v4l2.deb"

        if [ $? -ne 0 ]
        then
          echo 'Can not compile v4l2-utils. Abort!'
          exit 1
        fi

        echo 'v4l2-utils successfully compiled!'
    fi

    echo 'Started creating image...'
    docker build -t "$IMAGE_NAME" "$WORK_DIR"
    echo 'Done!'

    echo 'Compiling converter once...'
    docker rm -f "$CONTAINER_NAME" >/dev/null 2>&1
    docker run -d \
        -v /opt/vc:/opt/vc:ro \
        --name "$CONTAINER_NAME" \
        "$IMAGE_NAME"
    docker exec -it "$CONTAINER_NAME" /bin/bash -c 'cd converter && make'
    docker commit "$CONTAINER_NAME" "$IMAGE_NAME"
    docker rm -f "$CONTAINER_NAME" >/dev/null 2>&1
    echo 'Done!'
}

###########################
#### Start docker container ###
#########################
function start {
    if [ "$(docker images | egrep -c "$IMAGE_NAME")" -eq 0 ]; then
        echo "Can not find docker image. You should run 'build' command at first!"
        exit 1
    fi

    docker rm -f "$CONTAINER_NAME" >/dev/null 2>&1

    docker run -d \
        -e "VIDEO_DEV=$VIDEO_DEV" \
        -e "MAX_FILES_TO_KEEP=$MAX_FILES_TO_KEEP" \
        -p $1:80 \
        --device /dev/vchiq:/dev/vchiq \
        -v /opt/vc:/opt/vc:ro \
        -v /home/pi/RecordedData/Video:/RecordedData/Video \
        -v /home/pi/RecordedData/Images:/RecordedData/Images \
        -v /tmpfs/RaspiVideoRecorder:/RecordedData/Tmpfs \
        $(find /dev/ 2>/dev/null | egrep "/dev/video*" | xargs -I {} printf "--device={}:{} ") \
        --name "$CONTAINER_NAME" \
        "$IMAGE_NAME" >/dev/null 2>&1

    if [ $? -ne 0 ]
    then
      echo 'Fail!'
    else
      echo 'Success!'
    fi
}

# execute
main "$@"