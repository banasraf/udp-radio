#!/bin/bash

arecord -t raw -f cd | ./cmake-build-debug/sender -a 239.10.11.12 -P 15000 -C 10001
