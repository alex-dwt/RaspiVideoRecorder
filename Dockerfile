# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

FROM sdhibit/rpi-raspbian
MAINTAINER Alexander Lukashevich <aleksandr.dwt@gmail.com>

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget ca-certificates cron mkvtoolnix unzip \
    build-essential cmake pkg-config libswscale-dev \
    python3.4-dev python3-numpy \
    libjpeg-dev libpng-dev libtiff-dev libjasper-dev libjpeg8

RUN cd && wget https://github.com/opencv/opencv/archive/3.0.0.zip \
    && unzip 3.0.0.zip && rm -rf  3.0.0.zip && cd opencv-3.0.0 \
    && mkdir build && cd build && cmake .. && make -j4 \
    && make install && cd && rm -rf opencv-3.0.0

RUN cd && mkdir _node \
    && wget -O node.tar.xz https://nodejs.org/dist/v6.2.2/node-v6.2.2-linux-armv7l.tar.xz \
    && tar xvf node.tar.xz  -C _node && rm -f node.tar.xz \
    && cp -r _node/$(ls _node/)/* /usr/local/ \
    && rm -rf _node

ENV LD_LIBRARY_PATH /mjpg-streamer/
COPY ./build/* /mjpg-streamer/
RUN chmod +x /mjpg-streamer/*

RUN mkdir /recorder
COPY ./server/package.json /recorder/
WORKDIR /recorder
RUN npm install

COPY ./server /recorder/
RUN chmod +x ./script/*

CMD [ "/bin/bash", "/recorder/script/run.sh" ]