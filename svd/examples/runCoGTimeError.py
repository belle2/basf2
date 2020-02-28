#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from svd import *
from rawdata import add_unpackers
from simulation import add_simulation
from tracking import add_tracking_reconstruction
from ROOT import Belle2

from basf2 import conditions as b2conditions

# data = True
data = False

if data:
    b2conditions.override_globaltags()
    b2conditions.globaltags = ['klm_alignment_testing', 'online']

numEvents = 10

set_random_seed(1)
files = [' /group/belle2/dataprod/Data/Raw/e0010/r04925/sub00/beam.0010.04925.HLT*.root']

# old cDST
# files=["/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_hadron/cdst/sub00/cdst.physics.0010.04295.HLT1*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_bhabha/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_mumu_2trk/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root"]


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO


# Create paths
main = create_path()


# read input rootfile
if data:
    main.add_module("RootInput", inputFileNames=files)

else:
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    main.add_module(evtgeninput)

main.add_module('Gearbox')
main.add_module('Geometry')

if data:
    add_unpackers(main, components=['SVD', 'CDC'])
else:
    main.add_module('FullSim')
    add_simulation(main)

add_tracking_reconstruction(main, components=['SVD', 'CDC'])

# Select Strips form Tracks
fil = register_module('SVDShaperDigitsFromTracks')
fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
main.add_module(fil)

fitter = register_module('SVDCoGTimeEstimator')
fitter.set_name('post_SVDCoGTimeEstimator')
fitter.param('ShaperDigits', 'SVDShaperDigitsFromTracks')
fitter.param('RecoDigits', 'SVDRecoDigitsFromTracks')
main.add_module(fitter)

clusterizer = register_module('SVDSimpleClusterizer')
clusterizer.set_name('post_SVDSimpleClusterizer')
clusterizer.param('RecoDigits', 'SVDRecoDigitsFromTracks')
clusterizer.param('Clusters', 'SVDClustersFromTracks')
clusterizer.param('useDB', True)
main.add_module(clusterizer)

input_branches = [
    'EventT0',
    'SVDShaperDigits',
    'SVDShaperDigitsFromTracks',
    'SVDRecoDigits',
    'SVDRecoDigitsFromTracks',
    'SVDClusters',
    'SVDClustersFromTracks',
]

if data:
    input_branches.append('SVDEventInfo')
else:
    input_branches.append('SVDEventInfoSim')

if data:
    main.add_module("RootOutput", branchNames=input_branches, outputFileName="RootOutput_CoGerror_FromTracks_DATA.root")
else:
    main.add_module("RootOutput", branchNames=input_branches, outputFileName="RootOutput_CoGerror_FromTracks.root")

print_path(main)

# Process events
process(main)

print(statistics)
