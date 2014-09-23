#!/bin/sh
mkdir -p build
mkdir -p build/test
g++ -O3 -fno-inline-functions -std=gnu++11 -g -I$BOOST_ROOT -I. -I$CGUTIL_ROOT -DDAILY_ENABLE_INSTRUMENTATION=1 \
$1.cpp -o build/$1