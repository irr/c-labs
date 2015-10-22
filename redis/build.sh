#!/bin/bash
gcc -Werror -c -fpic -std=gnu99 geohash.c geohash_helper.c rgeohash.c -lm && ar -cvq librgeohash.a rgeohash.o geohash.o geohash_helper.o && rm -rf *.o && gcc -Werror -c -fpic -std=gnu99 geohash.c geohash_helper.c rgeohash.c -lm && gcc -shared -o librgeohash.so rgeohash.o geohash.o geohash_helper.o && rm -rf *.o 

