#include <stdio.h>

#include "geohash.h"
#include "geohash_helper.h"

int main()
{
    /*
        gcc -I/opt/nosql/redis-unstable/deps/geohash-int -L/opt/nosql/redis-unstable/deps/geohash-int -o test /opt/nosql/redis-unstable/deps/geohash-int/geohash.c test-geohash.c && ./test

        127.0.0.1:6379> geoadd irr 37.8324 12.5584 sp
        (integer) 1
        127.0.0.1:6379> geohash irr sp
        1) "sf4zkyhtyz0"
    */

    char *geoalphabet= "0123456789bcdefghjkmnpqrstuvwxyz";

    double xy[2] = { 37.8324, 12.5584 };

    GeoHashRange r[2];
    GeoHashBits hash;
    r[0].min = -180;
    r[0].max = 180;
    r[1].min = -90;
    r[1].max = 90;
    geohashEncode(&r[0],&r[1],xy[0],xy[1],26,&hash);

    char buf[12];
    int i;
    for (i = 0; i < 11; i++) {
        int idx = (hash.bits >> (52-((i+1)*5))) & 0x1f;
        buf[i] = geoalphabet[idx];
    }
    buf[11] = '\0';
    printf("lat: %.4f and lon: %.4f [%s]\n", xy[0], xy[1], buf);

    return 0;
}

