#!/bin/bash


for pid in $(ps -ef | awk '/a.out/ {print $2}'); do kill -9 $pid; done
for pid in $(ps -ef | awk '/UI/ {print $2}'); do kill -9 $pid; done
for pid in $(ps -ef | awk '/make/ {print $2}'); do kill -9 $pid; done
#for pid in $(ps -ef | awk '/make/ {print $2}'); do kill -9 $pid; done
#^figure out one for simpletestmess.py!!!
