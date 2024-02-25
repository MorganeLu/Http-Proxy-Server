#!/bin/bash
make clean
make
./proxyhttp &
while true ; do continue ; done