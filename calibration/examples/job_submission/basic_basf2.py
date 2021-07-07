#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Simple basf2 steering file that pulls in data files passed to it and does nothing
# with it. For use with the Backend examples

from basf2 import process, create_path, statistics
import sys
# The backend provides the input data files to the job and you can get the list of
# files from this function
from caf.backends import get_input_data

input_data = get_input_data()

main = create_path()
main.add_module("RootInput", inputFileNames=input_data)
main.add_module("Progress")
process(main)
print(statistics)
print("Testing that stderr works, no problems here.", file=sys.stderr)
