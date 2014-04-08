#!/bin/bash
make clean
rmmod cs_DADDR
rm /lib/modules/`uname -r`/cs_DADDR.ko
