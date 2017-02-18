# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

FROM sdhibit/rpi-raspbian
MAINTAINER Alexander Lukashevich <aleksandr.dwt@gmail.com>

ENV LD_LIBRARY_PATH /mjpg-streamer/
COPY ./build/* /mjpg-streamer/
RUN chmod +x /mjpg-streamer/*

RUN apt-get update && apt-get install -y --no-install-recommends wget ca-certificates libjpeg8-dev cron

RUN cd /tmp && mkdir _node && \
    wget -O node.tar.xz https://nodejs.org/dist/v6.2.2/node-v6.2.2-linux-armv7l.tar.xz && \
    tar xvf node.tar.xz  -C _node && cp -r _node/$(ls _node/)/* /usr/local/ && \
    rm -f node.tar.xz && rm -rf _node

RUN mkdir /recorder
COPY ./server/package.json /recorder/
WORKDIR /recorder
RUN npm install

COPY ./server /recorder/
RUN chmod +x ./script/*

CMD [ "/bin/bash", "/recorder/script/run.sh" ]