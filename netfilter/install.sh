#!/bin/bash
make
cp test.conf /etc/modprobe.d/
cp test.ko /lib/modules/`uname -r`/
depmod -a
modprobe test
echo 'modprobe test' >> /etc/rc.modules
chmod +x /etc/rc.modules
