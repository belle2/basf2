#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import Path, process
from sys import argv

main = Path()

main.add_module('RootInput')

masterclass = main.add_module('MasterClass')
if (len(argv) > 1):
    masterclass.param('outputFileName', argv[1])

process(main)
