#!/usr/bin/env python
# -*- coding: utf-8 -*-

#

from sys import *


def main():
    inner = []
    outer = []
    det = 0
    for updown in [0, 1]:
        layer = 0
        for nr in [
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
            ]:
            det = layer * 32 + nr * 2 + updown
            inner.append(det)

        layer = 1
        for nr in [
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12,
            ]:
            det = layer * 32 + nr * 2 + updown
            outer.append(det)

    inner.reverse()
    outer.reverse()

    print "packer.param('dhh_to_dhhc', ["
    for i in xrange(0, 8):
        print '  [', i, ',', inner.pop(), ',', inner.pop(), ',', outer.pop(), \
            ',', outer.pop(), ',', outer.pop(), '] ,'
    print '])'


main()

