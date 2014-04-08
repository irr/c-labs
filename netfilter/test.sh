#!/bin/bash
make
cp test.conf /etc/modprobe.d/
cp test.ko /lib/modules/`uname -r`/
depmod -a
modprobe test
tail -f /var/log/messages |grep test
