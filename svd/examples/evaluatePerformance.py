#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from simulation import add_simulation
from svd import *
from tracking import *
import glob

numEvents = 2000

'''
globalTag = "development"
reset_database()
use_database_chain()
use_central_database(globalTag)
'''

main = create_path()

set_random_seed(1)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
# bkgFiles = ""

ROIfinding = False

add_simulation(main, components=['MagneticField', 'BeamPipe', 'PXD', 'SVD'], bkgfiles=bkgFiles, usePXDDataReduction=ROIfinding)

add_svd_reconstruction(main)

add_tracking_reconstruction(
    main,
    components=["SVD"],
    use_vxdtf2=True,
    mcTrackFinding=True,
    additionalTrackFitHypotheses=[211],
    skipHitPreparerAdding=True)


tag = "_Y4S_wBKG_noROI_MCTF.root"
clseval = register_module('SVDClusterEvaluation')
clseval.param('outputFileName', "ClusterEvaluation" + str(tag))
main.add_module(clseval)

svdperf = register_module('SVDPerformance')
svdperf.param('outputFileName', "SVDPerformance" + str(tag))
main.add_module(svdperf)

# main.add_module('RootOutput')
main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
