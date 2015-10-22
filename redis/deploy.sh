#!/bin/bash
sudo mv librgeohash.* /usr/local/lib/ && sudo chown root:root /usr/local/lib/librgeohash.* && sudo chmod 755 /usr/local/lib/librgeohash.so && rm -rf *.o && sudo ldconfig && ldconfig -p |grep rgeohash && ls -alF /usr/local/lib/librgeohash.* 

