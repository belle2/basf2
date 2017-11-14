#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from simulation import add_simulation
from svd import *
from tracking import *
import glob

numEvents = 2000

globalTag = "development"
reset_database()
use_database_chain()
use_central_database(globalTag)

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
# bkgFiles = "/sw/belle2/bkg/twoPhoton_usual-phase3-optimized.root"
# bkgFiles = ""

add_simulation(main, components=['MagneticField', 'SVD'], bkgfiles=bkgFiles, usePXDDataReduction=False)

add_svd_reconstruction(main, isROIsimulation=False, useNN=False, useCoG=True)

# add_tracking_reconstruction(main, components=["SVD"], use_vxdtf2=True, mcTrackFinding=True,
#                            additionalTrackFitHypotheses=[211], skipHitPreparerAdding=True)

clseval = register_module('SVDClusterEvaluation')
clseval.param('outputFileName', "ClusterEvaluation_Y4S_wBKG_MC_CoG.root")
main.add_module(clseval)


# main.add_module('RootOutput')
main.add_module('Progress')
print_path(main)
process(main)

print(statistics)
