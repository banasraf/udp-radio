#!/bin/bash 

./cmake-build-debug/receiver -U 13333 -b 131072 -n "Moje radio" | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -
