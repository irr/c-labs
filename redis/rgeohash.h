#ifndef redis_geohash__h__
#define redis_geohash__h__

#include <stdint.h>

extern void geohash(double longitude, double latitude, char* hash);
extern uint64_t geohash64(double longitude, double latitude);

extern void geohash_decode(uint64_t bits, double* longitude, double* latitude);
extern uint64_t geohash_decode64(char* buf);

#endif  // redis_geohash__h__
