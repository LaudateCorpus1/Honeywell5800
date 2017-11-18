#!/bin/sh
#
# Set -DSHOW to write device state table to stdout when a state changes
# Set -DDEBUG for full debug output
#
# Build with no debug output
g++ -o honeywell --std=c++11 digitalDecoder.cpp analogDecoder.cpp main.cpp -lrtlsdr

# Build with data output to stdout
#g++ -o honeywell -DSHOW --std=c++11 digitalDecoder.cpp analogDecoder.cpp main.cpp -lrtlsdr

# Build with full debug output to stdout
#g++ -o honeywell -DDEBUG --std=c++11 digitalDecoder.cpp analogDecoder.cpp main.cpp -lrtlsdr
