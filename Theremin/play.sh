#!/bin/bash

#kill any existing processes

./kill.sh

#./a.out #RPI
cd Tone
#sudo python simpletestmess.py & #RPI
./a.out & #NOT RPI
make run &
cd ..

cd UI
./UI &
cd ..

