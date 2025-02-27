#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <output>../mixed.cdst.root</output>
    <description>creating mixed MC cdst files needed for the DecayFile/Multiplicity validation</description>
</header>
"""

import basf2 as b2
import generators as ge

b2.set_random_seed('mixed')

# create path
main = b2.create_path()

# default to early phase 3 (exp=1003), run 0, number of events = 100000
main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=100000)

# generate BBbar events
ge.add_evtgen_generator(path=main, finalstate='mixed')

# finally add cdst output
main.add_module('RootOutput', outputFileName='../mixed.cdst.root')

main.add_module('Progress')
# process events and print call statistics
b2.process(path=main)
print(b2.statistics)
