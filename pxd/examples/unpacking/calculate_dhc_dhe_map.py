#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
#
# This script dumps out a valid DHC-DHE mapping table for simulation.
# As we do not have an idea what the real mapping will be, take as an
# workaround, until we have the table in data base.
#
##############################################################################


def main():
    inner = []
    outer = []
    det = 0
    for updown in [0, 1]:
        layer = 0
        for nr in [1, 2, 3, 4, 5, 6, 7, 8]:
            det = layer * 32 + nr * 2 + updown
            inner.append(det)

        layer = 1
        for nr in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]:
            det = layer * 32 + nr * 2 + updown
            outer.append(det)

    inner.reverse()
    outer.reverse()

    print("packer.param('dhe_to_dhc', [")
    for i in range(0, 8):
        print('  [', i, ',', inner.pop(), ',', inner.pop(), ',', outer.pop(), ',', outer.pop(), ',', outer.pop(), '] ,')
    print('])')


main()
