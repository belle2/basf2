#!/bin/sh
test -d /dev/copper || mkdir /dev/copper
test -e /dev/copper/fngeneric:a  || mknod /dev/copper/fngeneric:a  c 210 0 
test -e /dev/copper/fngeneric:b  || mknod /dev/copper/fngeneric:b  c 210 1 
test -e /dev/copper/fngeneric:c  || mknod /dev/copper/fngeneric:c  c 210 2 
test -e /dev/copper/fngeneric:d  || mknod /dev/copper/fngeneric:d  c 210 3 
