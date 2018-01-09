#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from simulation import add_simulation
from svd import *
from tracking import *
import glob

numEvents = 2000

bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
# bkgFiles = None #if no background
simulateJitter = False
ROIfinding = False
Phase2 = False

main = create_path()

set_random_seed(1)

expList = [0]
if Phase2:
    expList = [1002]

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', expList)
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

add_simulation(
    main,
    components=[
        'MagneticField',
        'BeamPipe',
        'PXD',
        'SVD'],
    bkgfiles=bkgFiles,
    usePXDDataReduction=ROIfinding,
    simulateT0jitter=simulateJitter)

add_svd_reconstruction(main)

add_tracking_reconstruction(
    main,
    components=["SVD"],
    use_vxdtf2=True,
    mcTrackFinding=True,
    additionalTrackFitHypotheses=[211],
    skipHitPreparerAdding=True)


tag = "_Y4S_noJitter_noBKG_noROI_MCTF.root"
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
