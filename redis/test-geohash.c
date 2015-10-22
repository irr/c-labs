#include <stdio.h>
#include "rgeohash.h"

/*

SHARED:
gcc -std=gnu99 -I/home/irocha/c/redis -L/home/irocha/c/redis -Wall -o test test-geohash.c -lrgeohash -lm && LD_LIBRARY_PATH=/home/irocha/c/redis ./test

STATIC:
gcc -std=gnu99 -static -I/home/irocha/c/redis -L/home/irocha/c/redis -Wall -o test test-geohash.c -lrgeohash -lm && ./test

rm -rf *.o *.so *.a test

127.0.0.1:6379> geoadd irr -46.691657 -23.570018 sp
(integer) 1
127.0.0.1:6379> geohash irr sp
1) "6gycct0ntw0"

http://geohash.org/6gycct0ntw0

extern void geohash(double longitude, double latitude, char* hash);
extern uint64_t geohash64(double longitude, double latitude);

extern void geohash_decode(uint64_t bits, double* longitude, double* latitude) {
extern uint64_t geohash_decode64(char* buf);

*/

int main() {
    double longitude = -46.691657;
    double latitude = -23.570018;
    char hash[12];
    uint64_t bits;

    geohash(longitude, latitude, hash);
    bits = geohash64(longitude, latitude);    

    printf("geohash...\n");
    printf("longitude: %.8f and latitude: %.8f [%s] = %ld\n", longitude, latitude, hash, bits);

    printf("geohash (decode)...\n");
    geohash_decode(bits, &longitude, &latitude);
    bits = geohash_decode64(hash);
    printf("longitude: %.8f and latitude: %.8f [%s] = %ld\n", longitude, latitude, hash, bits);

    return 0;
}

