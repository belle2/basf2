#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# copy a beam background simulatio file, keeping only data related to SVD.

import basf2 as b2

main = b2.create_path()

input = b2.register_module('RootInput')
input.param('branchNames', ['SVDSimHits', 'SVDTrueHits',
                            'SVDTrueHitsToSVDSimHits', 'BeamBackHits'])
# enter the path to the input data file(s) on your system here
input.param('inputFileName', '/data/belle2/BG/Jun2014/bg_full/Touschek_LER_100us.root')
main.add_module(input)

bbfilter = b2.register_module('SVDBeamBackHitFilter')
bbfilter.set_log_level(b2.LogLevel.INFO)
main.add_module(bbfilter)

output = b2.register_module('RootOutput')
# enter the path to the output file on your filesystem here
output.param('outputFileName', 'Touschek_LER_100us_SVD.root')
main.add_module(output)


main.add_module(b2.register_module('ProgressBar'))


b2.process(main)

print(b2.statistics)
