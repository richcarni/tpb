TPB=/usr/bin/tpb
CONFIG=/etc/default/tpb

if [ -e $CONFIG ]; then
  . $CONFIG
fi

if [ -x $TPB -a "$START_TPB" = true ]; then
  $TPB -d
fi
