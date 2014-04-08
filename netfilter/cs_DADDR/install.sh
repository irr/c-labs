#!/bin/bash
make
cp cs_DADDR.ko /lib/modules/`uname -r`/
depmod -a
modprobe cs_DADDR
echo 'modprobe cs_DADDR' >> /etc/rc.modules
chmod +x /etc/rc.modules
