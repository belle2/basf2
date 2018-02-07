#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10k Y(4S) events                        #
#                                                               #
#    written by Giulia Casarosa, INFN Pisa                      #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
# USAGE 1:
#
# > mkdir test
# > basf2 -n100 a0123_trackingEfficiency_oneshot.py 1 test noROI noBKG
#
# USAGE 2:
# > mkdir release/workdir
# > cd release/workdir
# > mkdir /group/belle2/users/casarosa/trackingValidation/release
# > ln -s /group/belle2/users/casarosa/trackingValidation/release .
# > ln -s release/tracking/examples/TrackingPerformanceEvaluation_scripts/a*.py
# > bsub -q s -o oneshot_release_1.out basf2 a0123_trackingEfficiency_oneshot.py roi bkg vxdtf
#
#################################################################

import sys
import glob
from basf2 import *
from simulation import add_simulation
from tracking import *
from modularAnalysis import *

set_random_seed(1509)

particleGun = False

release = sys.argv[1]

# roi = {noROI, vxdtf1, vxdtf2}
# bkg = {noBkg, stdBKG, std2GBKG}
# vxdtf = {vxdtf1, vxdtf2}

roi = sys.argv[2]
bkg = sys.argv[3]
vxdtf = sys.argv[4]

release = 'merged'

print('Performance evaluated on: ')
print('simulation: ' + roi + ' ' + bkg)
print('reconstruction: ' + vxdtf)
print()

root_file_name_TRK = './' + release + '/oneshotTV_TRK_analysis_' + roi + '_' + bkg + '_' + vxdtf + '_' + release + '.root'
root_file_name_V0 = './' + release + '/oneshotTV_V0_analysis_' + roi + '_' + bkg + '_' + vxdtf + '_' + release + '.root'

bkgFiles = None
usePXDDataReduction = True
mcTrackFinding = False

if roi == 'noROI':
    usePXDDataReduction = False

if bkg == 'stdBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
if bkg == 'std2GsmallBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*usual.root')
if bkg == 'std2GBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*.root')

print(bkgFiles)

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1])
eventinfosetter.param('runList', [1])

progress = register_module('Progress')

v0matcher = register_module('MCV0Matcher')
v0matcher.param('V0ColName', 'V0ValidationVertexs')
v0matcher.logging.log_level = LogLevel.INFO

create_plots_TRK = register_module('TrackingPerformanceEvaluation')
create_plots_TRK.param('outputFileName', root_file_name_TRK)
create_plots_TRK.logging.log_level = LogLevel.INFO


create_plots_V0 = register_module('V0findingPerformanceEvaluation')
create_plots_V0.param('outputFileName', root_file_name_V0)
create_plots_V0.logging.log_level = LogLevel.INFO

if particleGun:
    particleGunModule = register_module('ParticleGun')
    particleGunModule.param({
        'pdgCodes': [211, -211],
        'nTracks': 1,
        'varyNTracks': False,
        'momentumGeneration': 'uniformpt',
        'momentumParams': [0.5, 1.],
        'thetaGeneration': 'uniform',
        'thetaParams': [60., 60.],
    })
    path.add_module('EventInfoSetter')
    path.add_module(particleGunModule)
else:
    generateY4S(100, None, path)

path.add_module(progress)

add_simulation(path, None, bkgFiles, None, 1.0, usePXDDataReduction)

add_tracking_reconstruction(
    path,
    components=None,
    pruneTracks=False,
    mcTrackFinding=mcTrackFinding,
    trigger_mode="all",
    skipGeometryAdding=False
)


modList = path.modules()
for modItem in modList:
    if modItem.name() == 'V0Finder':
        modItem.param('Validation', True)
path.add_module(v0matcher)

path.add_module(create_plots_TRK)
path.add_module(create_plots_V0)

process(path)

print(statistics)
