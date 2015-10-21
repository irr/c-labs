#include <stdio.h>
#include <math.h>
#include <string.h>

#include "geohash.h"
#include "geohash_helper.h"

char *geoalphabet= "0123456789bcdefghjkmnpqrstuvwxyz";

void geohash(double longitude, double latitude, char* hash) {
    GeoHashRange r[2];
    GeoHashBits h;

    geohashEncodeWGS84(longitude, latitude, GEO_STEP_MAX, &h);

    r[0].min = -180;
    r[0].max = 180;
    r[1].min = -90;
    r[1].max = 90;

    geohashEncode(&r[0], &r[1], longitude, latitude, 26, &h);

    for (int i = 0; i < 11; i++) {
        int idx = (h.bits >> (52-((i+1)*5))) & 0x1f;
        hash[i] = geoalphabet[idx];
    }

    hash[11] = '\0';
}

uint64_t geohash64(double longitude, double latitude) {
    GeoHashBits h;
    geohashEncodeWGS84(longitude, latitude, GEO_STEP_MAX, &h);
    
    return geohashAlign52Bits(h);
}

uint64_t geohash_decode64(char* buf) {
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

void geohash_decode(uint64_t bits, double* longitude, double* latitude) {
    GeoHashBits h = { .bits = (uint64_t)bits, .step = GEO_STEP_MAX };
    double xy[2];
    geohashDecodeToLongLatWGS84(h, xy);
    (*longitude) = xy[0]; 
    (*latitude) = xy[1]; 
}

/*
gcc -c -fpic -std=gnu99 -I/opt/nosql/redis-unstable/deps/geohash-int -L/opt/nosql/redis-unstable/deps/geohash-int /opt/nosql/redis-unstable/deps/geohash-int/geohash.c /opt/nosql/redis-unstable/deps/geohash-int/geohash_helper.c redis-geohash.c -lm && gcc -shared -o libredis-geohash.so redis-geohash.o
*/

