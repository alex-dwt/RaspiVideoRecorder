#RaspiVideoRecorder

##General description
RaspiVideoRecorder is a client-server application for recording video from web-cameras connected to raspberry pi.

##Setup
Recorder should write to tmpfs-partition at first (because it is very fast),
So you should create partition by "# printf "\ntmpfs /tmpfs tmpfs defaults,noatime,noexec,size=100M 0 0\n" >> /etc/fstab".

##License
RaspiVideoRecorder is under the MIT license.