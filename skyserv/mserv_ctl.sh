#!/bin/sh

CFGDIR=$HOME/gateways
SCRN=2000
# XVFB=/usr/bin/Xvfb -screen 2 120x120x16 :2
XVFB=/usr/bin/Xvfb
OBOX=/usr/bin/openbox
X11VNC=/usr/bin/x11vnc
SKYPE=/usr/bin/skype

## start
$XVFB :$SCRN &
sleep 2
DISPLAY=:$SCRN
$OBOX &
echo drswinghead 2032103 | $SKYPE --pipelogin --dbpath=$CFGDIR/sd${SCRN} &
$X11VNC &

echo "DISPLAY=:${SCRN} ./skyserv"

echo "vncview localhost:${SCRN}"

echo "killall -9 Xvfb"


