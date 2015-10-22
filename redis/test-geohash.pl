use Modern::Perl;
use Data::Dumper;

use Inline C => <<'END_C', LIBS => '-lrgeohash'; 

#include <stdint.h>
#include <string.h>
#include "rgeohash.h"

void geohash_sv(double longitude, double latitude, SV* res) {
    char hash[12];
    geohash(longitude, latitude, hash);
    sv_setpvn(res, hash, strlen(hash));
}

unsigned long geohash64_sv(double longitude, double latitude) {
    return (unsigned long) geohash64(longitude, latitude);
}

void geohash_decode_sv(unsigned long bits, SV* longitude, SV* latitude) {
    double lon, lat;
    geohash_decode(bits, &lon, &lat);
    sv_setnv(longitude, lon);
    sv_setnv(latitude, lat);
}


unsigned long geohash_decode64_sv(char* buf) {
    return (unsigned long) geohash_decode64(buf);
}

END_C

my ($hash, $hash64, $longitude, $latitude, $hash64_sv);

geohash_sv(-46.691657, -23.570018, $hash);

$hash64 = geohash64_sv(-46.691657, -23.570018);

geohash_decode_sv($hash64 + 0, $longitude, $latitude);

$hash64_sv = geohash_decode64_sv($hash);

say(Dumper([$hash, $hash64, $longitude, $latitude, $hash64_sv], 
           [qw(hash hash64 longitude latitude hash64_sv)]));
