#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from simulation import add_simulation
from svd import add_svd_reconstruction
from tracking import add_tracking_reconstruction
import glob

numEvents = 2000

bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
bkgFiles = None  # uncomment to remove  background
simulateJitter = False

ROIfinding = False
Phase2 = False
MCTracking = True
# set this string to identify the output rootfiles
tag = "_Y4S_noJitter_noBKG_noROI_MCTF.root"

main = b2.create_path()

b2.set_random_seed(1)

expList = [0]
if Phase2:
    expList = [1002]

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', expList)
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

add_simulation(
    main,
    bkgfiles=bkgFiles,
    usePXDDataReduction=ROIfinding,
    simulateT0jitter=simulateJitter)

add_svd_reconstruction(main)

'''
add_tracking_reconstruction(
    main,
    components=["SVD"],
    mcTrackFinding=MCTracking,
    trackFitHypotheses=[211],
    skipHitPreparerAdding=True)


tag = "_Y4S_jitter10ns_wBKG_noROI_MCTF.root"
clseval = b2.register_module('SVDClusterEvaluationTrueInfo')
clseval.param('outputFileName', "ClusterEvaluationTrueInfo" + str(tag))
main.add_module(clseval)

svdperf = b2.register_module('SVDPerformance')
svdperf.param('outputFileName', "SVDPerformance" + str(tag))
main.add_module(svdperf)
'''
# main.add_module('RootOutput')
main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
