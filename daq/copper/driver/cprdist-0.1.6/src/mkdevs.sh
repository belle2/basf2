#!/bin/sh
test -d /dev/copper || mkdir /dev/copper
test -e /dev/copper/plx9054 || mknod /dev/copper/plx9054 c 198 0 
test -e /dev/copper/copper || mknod /dev/copper/copper c 199 0 
test -e /dev/copper/copper_ctrl || mknod /dev/copper/copper_ctrl c 199 1
test -e /dev/copper/amt3:a || mknod /dev/copper/amt3:a c 200 0 
test -e /dev/copper/amt3:b || mknod /dev/copper/amt3:b c 200 1 
test -e /dev/copper/amt3:c || mknod /dev/copper/amt3:c c 200 2 
test -e /dev/copper/amt3:d || mknod /dev/copper/amt3:d c 200 3 
test -e /dev/copper/jig:a  || mknod /dev/copper/jig:a  c 200 0 
test -e /dev/copper/jig:b  || mknod /dev/copper/jig:b  c 200 1 
test -e /dev/copper/jig:c  || mknod /dev/copper/jig:c  c 200 2 
test -e /dev/copper/jig:d  || mknod /dev/copper/jig:d  c 200 3 
test -e /dev/copper/fngeneric:a  || mknod /dev/copper/fngeneric:a  c 210 0 
test -e /dev/copper/fngeneric:b  || mknod /dev/copper/fngeneric:b  c 210 1 
test -e /dev/copper/fngeneric:c  || mknod /dev/copper/fngeneric:c  c 210 2 
test -e /dev/copper/fngeneric:d  || mknod /dev/copper/fngeneric:d  c 210 3 
chmod a+w /dev/copper/fngeneric:a
chmod a+w /dev/copper/fngeneric:b
chmod a+w /dev/copper/fngeneric:c
chmod a+w /dev/copper/fngeneric:d
