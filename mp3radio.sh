#!/bin/bash

sox -S $1 -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./cmake-build-debug/sender -a 239.10.11.12 -n "MP3 Radio"
