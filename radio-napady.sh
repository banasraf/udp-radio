#!/bin/bash

sox -S "Napady.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./cmake-build-debug/sender -a 239.10.11.12 -P 15000 -C 10001 -n "Radio Napady"
