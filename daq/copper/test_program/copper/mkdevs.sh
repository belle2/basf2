#!/bin/sh
test - d / dev / copper || mkdir / dev / copper
test - e / dev / copper / plx9054 || mknod / dev / copper / plx9054 c 198 0
test - e / dev / copper / copper || mknod / dev / copper / copper c 199 0
test - e / dev / copper / amt3: a || mknod / dev / copper / amt3: a c 200 0
test - e / dev / copper / amt3: b || mknod / dev / copper / amt3: b c 200 1
test - e / dev / copper / amt3: c || mknod / dev / copper / amt3: c c 200 2
test - e / dev / copper / amt3: d || mknod / dev / copper / amt3: d c 200 3
test - e / dev / copper / jig: a  || mknod / dev / copper / jig: a  c 200 0
test - e / dev / copper / jig: b  || mknod / dev / copper / jig: b  c 200 1
test - e / dev / copper / jig: c  || mknod / dev / copper / jig: c  c 200 2
test - e / dev / copper / jig: d  || mknod / dev / copper / jig: d  c 200 3
