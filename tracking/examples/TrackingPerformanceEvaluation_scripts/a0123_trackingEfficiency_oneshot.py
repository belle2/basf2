#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10k Y(4S) events                        #
#                                                               #
#    written by Giulia Casarosa, INFN Pisa                      #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
# USAGE:
#
# > mkdir release/workdir
# > cd release/workdir
# > mkdir /group/belle2/users/casarosa/trackingValidation/release
# > ln -s /group/belle2/users/casarosa/trackingValidation/release .
# > bsub -q s -o oneshot_release_1.out basf2 \
#    ../tracking/examples/TrackingPerformanceEvaluation_scripts/a0123_trackingEfficiency_oneshot.py seed release ROI BKG
#
#################################################################

import sys
import glob
from basf2 import *
from simulation import add_simulation
from tracking import *  # add_tracking_reconstruction
from modularAnalysis import *  # add_tracking_reconstruction

set_random_seed(123 + int(sys.argv[1]))

release = sys.argv[2]

roi = sys.argv[3]
bkg = sys.argv[4]

print(release)
print(roi)
print(bkg)

root_file_name_TRK = './' + release + '/TV_TRK_analysis_' + roi + '_' + bkg + '_' + release + '.root'
root_file_name_V0 = './' + release + '/TV_V0_analysis_' + roi + '_' + bkg + '_' + release + '.root'

bkgFiles = None
usePXDDataReduction = False

if roi == 'wROI':
    usePXDDataReduction = True

if bkg == 'stdBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
if bkg == 'std2GsmallBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*usual.root')
if bkg == 'std2GBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*.root')

print(bkgFiles)

# print(input_filename)

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1])
eventinfosetter.param('runList', [1])
# eventinfosetter.param('evtNumList', [200])

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

generateY4S(100, None, path)

path.add_module(progress)

add_simulation(path, None, bkgFiles, None, 1.0, usePXDDataReduction)

add_tracking_reconstruction(
    path,
    components=None,
    pruneTracks=False,
    mcTrackFinding=False,
    trigger_mode="all",
    skipGeometryAdding=False)
tfmctruth = register_module('TrackFinderMCTruthRecoTracks')
tfmctruth.param('RecoTracksStoreArrayName', 'MCRecoTracks')

path.add_module(tfmctruth)
add_mc_matcher(path)

genfitTrackCand = register_module('GenfitTrackCandidatesCreator')
path.add_module(genfitTrackCand)

genfitMCTrackCand = register_module('GenfitTrackCandidatesCreator')
genfitMCTrackCand.set_name('GenfitMCTrackCandidatesCreator')
genfitMCTrackCand.param('recoTracksStoreArrayName', 'MCRecoTracks')
genfitMCTrackCand.param('genfitTrackCandsStoreArrayName', 'MCGFTrackCands')
path.add_module(genfitMCTrackCand)

path.add_module('GenfitTrackCreator')

modList = path.modules()
for modItem in modList:
    if modItem.name() == 'V0Finder':
        modItem.param('Validation', True)

# path.add_module(mctf)
# path.add_module(matcher)
path.add_module(v0matcher)

path.add_module(create_plots_TRK)
path.add_module(create_plots_V0)

# path.add_module(root_output)

# for modItem in modList:
#    if modItem.name() == 'PruneGenfitTracks':
#        modItem.m_flags = ''

process(path)
print(statistics)
