#!/bin/sh
mkdir -p build
mkdir -p build/test
g++ -O3 -std=gnu++11 -I$BOOST_ROOT -I. \
$1.cpp -o build/$1