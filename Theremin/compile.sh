#!/bin/bash

#gcc led_code.c -lwiringPi

cd Tone/
make clean
make
rm a.out
g++ theresim.cpp #not RPI
cd ..

cd UI/
make clean
make
cd ..
