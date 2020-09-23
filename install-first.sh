#!/bin/bash
PORT=`arduino-cli board list | grep $1 -m1 | awk -F' ' '{print $1}'`
echo "Port: $PORT"
arduino-cli upload -p $PORT -b arduino:avr:leonardo
