#!/usr/bin/env luajit

local ffi = require("ffi")

local rgeo = ffi.load("librgeohash.so")

ffi.cdef[[
void geohash(double longitude, double latitude, char* hash);
uint64_t geohash64(double longitude, double latitude);
void geohash_decode(uint64_t bits, double* longitude, double* latitude);
uint64_t geohash_decode64(char* buf);
]]

local function geohash(longitude, latitude)
    local lon = ffi.new("double", longitude)
    local lat = ffi.new("double", latitude)
    local hash = ffi.new("char[?]", 12)
    rgeo.geohash(lon, lat, hash)
    return ffi.string(hash, 11)
end

local function geohash64(longitude, latitude)
    return rgeo.geohash64(longitude, latitude)
end

local function geohash_decode(bits)
    local b = ffi.new("uint64_t", bits)
    local longitude = ffi.new("double[1]", 10.1)
    local latitude = ffi.new("double[1]", 20.2)
    rgeo.geohash_decode(b, longitude, latitude)
    return longitude[0], latitude[0]
end

local function geohash_decode64(hash)
    local h = ffi.new("char[12]", hash)
    return rgeo.geohash_decode64(h)
end

local longitude = -46.691657
local latitude = -23.570018

local hash = geohash(longitude, latitude)
local bits = geohash64(longitude, latitude)

print("geohash...");
print(string.format("longitude: %.8f and latitude: %.8f [%s] = %s\n", longitude, latitude, hash, bits))

longitude, latitude = geohash_decode(bits)
bits = geohash_decode64(hash)

print("geohash (decode)...")
print(string.format("longitude: %.8f and latitude: %.8f [%s] = %s\n", longitude, latitude, hash, bits))

print("testing...")
print(string.format("1.invalid longitude/latitude [%s]", geohash(1, 1111)));


