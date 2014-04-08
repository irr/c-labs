#!/bin/bash
make clean
rmmod test
rm /etc/modprobe.d/test.conf 
rm /lib/modules/`uname -r`/test.ko
