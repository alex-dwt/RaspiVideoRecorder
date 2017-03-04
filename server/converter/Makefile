# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

DMX_INC = -I /opt/vc/include/ -I /opt/vc/include/interface/vmcs_host/ -I /opt/vc/include/interface/vcos/pthreads -I /opt/vc/include/interface/vmcs_host/linux
EGL_INC =
OMX_INC = -I /opt/vc/include/IL
OMX_ILCLIENT_INC = -I /opt/vc/src/hello_pi/libs/ilclient
INCLUDES = $(DMX_INC) $(EGL_INC) $(OMX_INC) $(OMX_ILCLIENT_INC)
CFLAGS=-g -DRASPBERRY_PI -DOMX_SKIP64BIT $(INCLUDES)
CPPFLAGS =
DMX_LIBS = -L /opt/vc/lib/ -lbcm_host -lvcos -lvchiq_arm -lpthread
EGL_LIBS = -L /opt/vc/lib/ -lEGL -lGLESv2
OMX_LIBS = -lopenmaxil
OMX_ILCLIENT_LIBS = -L /opt/vc/src/hello_pi/libs/ilclient -lilclient
LDFLAGS="-Wl,-rpath=/opt/vc/lib/"
LDLIBS = $(DMX_LIBS) $(EGL_LIBS) $(OMX_LIBS) $(LDFLAGS) $(OMX_ILCLIENT_LIBS)
all: converter