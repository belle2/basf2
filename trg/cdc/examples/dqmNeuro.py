##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import neurotrigger
import reconstruction
import sys
import os
import rawdata

################################################################################
# Setting global tags in case Raw data is unpacked: ###

# basf2.conditions.override_globaltags()
# basf2.conditions.append_globaltag('klm_alignment_testing')
# basf2.conditions.append_globaltag('neurotrigger')  # should not be needed
# basf2.conditions.append_globaltag('online')

################################################################################
# Optional Log Output:
basf2.set_log_level(basf2.LogLevel.DEBUG)
basf2.set_debug_level(20)


################################################################################
# Start path: ###
################################################################################
main = basf2.create_path()

# Loading filelist and checking for ending: ###
dstfiles = [sys.argv[1]]
ending = ''
if ".sroot" in dstfiles[0]:
    ending = ".sroot"
else:
    ending = ".root"

print('Files to be processed:')
for x in dstfiles:
    print(x)

# Creating output directories with the script's name: ###
os.makedirs(sys.argv[0].split('.py')[0] + '/data', exist_ok=True)
os.makedirs(sys.argv[0].split('.py')[0] + '/hist', exist_ok=True)

# Creating filenames for dqm histogram output and root dst output: ###
outputfile = sys.argv[0].split('.py')[0] + '/hist/histo.' + sys.argv[1].split('/')[-1].split(ending)[0] + '.root'
dstputfile = sys.argv[0].split('.py')[0] + '/data/dst.' + sys.argv[1].split('/')[-1].split(ending)[0] + '.root'

# adding root input module depending on the input file: ###
if ending == ".sroot":
    main.add_module("SeqRootInput", inputFileNames=dstfiles)
else:
    main.add_module("RootInput", inputFileNames=dstfiles)

# loading gearbox and geometry, which is needed for simulation: ###
main.add_module('Gearbox')
main.add_module('Geometry')

# show progress at least every 10^maxN events: ###
main.add_module('Progress', maxN=3)

# add unpacker function from the rawdata script or the neurotrigger script: ###
if ending == ".sroot":
    rawdata.add_unpackers(main)
else:
    neurotrigger.add_neuro_2d_unpackers(main)

# add filter to just use events with trg information present: ###
main.add_module(neurotrigger.nnt_eventfilter())  # branchname="CDCTriggerNNInput2DFinderTracks"))

# adding neurotrigger simulations for one hwsim and one swsim case: ###
# neurotrigger.add_neurotrigger_sim(main)
neurotrigger.add_neurotrigger_hw(main)

# adding software neurotrigger simulation from CDCHits on:
# neurotrigger.add_neuro_simulation(main)

# add reconstruction in case .sroot files were used: ###
#    main.add_module('CDCUnpacker')
if ending == ".sroot":
    reconstruction.add_reconstruction(main, add_trigger_calculation=False)

# add matcher modules to match trigger tracks to reco tracks: ###
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwneurotracks,
                hitCollectionName=neurotrigger.hwneuroinputsegmenthits, axialOnly=True)
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwsimneurotracks,
                hitCollectionName=neurotrigger.hwneuroinputsegmenthits, axialOnly=True)
main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.hwneuroinput2dfindertracks,
                hitCollectionName=neurotrigger.hwneuroinputsegmenthits, axialOnly=True)
# main.add_module('CDCTriggerRecoHitMatcher', hitCollectionName=neurotrigger.simsegmenthits)
# main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=neurotrigger.simneurotracks_swtssw2d,
#                hitCollectionName=neurotrigger.simsegmenthits, axialOnly=True)
main.add_module('SetupGenfitExtrapolation')

# adding histomanager and the dqm module: ###
main.add_module('HistoManager',
                histoFileName=outputfile)
main.add_module('CDCTriggerNeuroDQM',
                simNeuroTracksName=neurotrigger.hwsimneurotracks,
                unpackedNeuroInput2dTracksName=neurotrigger.hwneuroinput2dfindertracks,
                # simNeuroTracksSWTSSW2DName=neurotrigger.simneurotracks_swtssw2d,
                # sim2DTracksSWTSName=neurotrigger.sim2dtracks_swts,
                # simSegmentHitsName=neurotrigger.simsegmenthits,
                showRecoTracks=True,
                skipWithoutHWTS=True,
                maxRecoZDist=-1,
                maxRecoD0Dist=-1,
                limitedoutput=False,
                )

# add root output: ###
main.add_module('RootOutput', outputFileName=dstputfile)

# run basf2: ###
basf2.process(main)
print(basf2.statistics)
