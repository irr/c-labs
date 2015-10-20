#include <stdio.h>
#include <math.h>
#include <string.h>

#include "geohash.h"
#include "geohash_helper.h"

char *geoalphabet= "0123456789bcdefghjkmnpqrstuvwxyz";

uint64_t decode(char* buf) {
    double coords[2][2] = { { -90.0, 90.0 }, 
                            { -180.0, 180.0 } };
    int bits[] = { 16, 8, 4, 2, 1 };
    int flip = 1;

    for (int i = 0; i < 11; i++) {
        char* pch = strchr(geoalphabet, buf[i]);
        if (pch == NULL) return 0;
        int pos = pch - geoalphabet;
        for (int j = 0; j < 5; j++) {
            coords[flip][((pos & bits[j]) > 0) ? 0 : 1] = (coords[flip][0] + coords[flip][1]) / 2.0;
            flip = !flip;
        }
    }

    GeoHashBits hash;
    geohashEncodeWGS84(*coords[1,1], *coords[1,0], GEO_STEP_MAX, &hash);
    return geohashAlign52Bits(hash);
}

int main() {
    /*
     gcc -std=gnu99 -I/opt/nosql/redis-unstable/deps/geohash-int -L/opt/nosql/redis-unstable/deps/geohash-int -o test /opt/nosql/redis-unstable/deps/geohash-int/geohash.c /opt/nosql/redis-unstable/deps/geohash-int/geohash_helper.c test-geohash.c -lm && ./test && rm -rf test

        127.0.0.1:6379> geoadd irr -46.691657 -23.570018 sp
        (integer) 1
        127.0.0.1:6379> geohash irr sp
        1) "6gycct0ntw0"

        http://geohash.org/6gycct0ntw0
    */

    // double xy[2] = { latitude, longitude };
    double xy[2] = { -46.691657, -23.570018 };

    GeoHashRange r[2];
    GeoHashBits hash;

    geohashEncodeWGS84(xy[0], xy[1], GEO_STEP_MAX, &hash);
    GeoHashFix52Bits bits = geohashAlign52Bits(hash);

    r[0].min = -180;
    r[0].max = 180;
    r[1].min = -90;
    r[1].max = 90;

    geohashEncode(&r[0],&r[1],xy[0],xy[1],26,&hash);

    char buf[12];
    for (int i = 0; i < 11; i++) {
        int idx = (hash.bits >> (52-((i+1)*5))) & 0x1f;
        buf[i] = geoalphabet[idx];
    }
    buf[11] = '\0';

    GeoHashBits h = { .bits = (uint64_t)bits, .step = GEO_STEP_MAX };

    double res[2];
    geohashDecodeToLongLatWGS84(h, res);

    printf("latitude: %.4f and longitude: %.4f [%s] = %ld\n", res[0], res[1], buf, bits);

    printf("decoded->uint_64t [%s] = %ld\n", buf, decode(buf));

    return 0;
}

