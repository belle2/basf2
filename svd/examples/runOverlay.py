#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
#
# Test script to produce SVD DQM plots
#
# Usage: basf2 runOverlay.py [cosmics, xTalk, user-defined]
#        user-defined rootfiles are also usable
#        but you need to specify the path to the rootfiles as
#        third argument of overlay_svd_data()
#
#############################################################

import sys
import basf2 as b2
import svd.overlay_utils as svdou
import simulation as simu
import glob

tag = "unused"
if len(sys.argv) == 2:
    tag = sys.argv[1]

'''
# PREPARE YOUR INPUT FILES - ERROR printed at the end does not
#                            affect output files
# function provides output rootfile with SVDShaperDigits only

# random TRG
# 1. link the input raw data in /gpfs/fs02/belle2/group/detector/SVD/overlayFiles/randomTRG/
# and select:
# location="/gpfs/fs02/belle2/group/detector/SVD/overlayFiles/randomTRG/*.root"
# and outputFileTag = ZS3
# 2. then select:
# location="/gpfs/fs02/belle2/group/detector/SVD/overlayFiles/randomTRG/*_ZS3.root"
# and outputFileTag = overlay
# and outputFileTag = overlayZS with same location

filelist=glob.glob(location)
# print(filelist)
for inputfile in filelist:
    main = b2.create_path()
#    svdou.prepare_svd_overlay(main, [inputfile],"ZS3")
    svdou.prepare_svd_overlay(main, [inputfile],"overlay")
#    svdou.prepare_svd_overlay(main, [inputfile],"overlayZS5")
'''

# EXAMPLE OF OVERLAY
main = b2.create_path()

b2.set_random_seed(1)

# set the exp/run event informations
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])
main.add_module(eventinfosetter)

# register HistoModules for DQM plots
main.add_module("HistoManager", histoFileName="SVDDQMOutput_"+str(tag)+".root")

# generate signal
main.add_module('EvtGenInput')

# add default simulation
bkgDir = '/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root'
bg = glob.glob(bkgDir)
if len(bg) == 0:
    b2.B2ERROR('No files found in ', bkgDir)
    sys.exit()
simu.add_simulation(main, bkgfiles=bg, usePXDDataReduction=False, forceSetPXDDataReduction=True)

if str(tag) == "xTalk" or str(tag) == "cosmics" or str(tag) == "randomTrigger" or str(tag) == "randomTriggerZS5":
    svdou.overlay_svd_data(main, str(tag))


# add offline ZS for DQM
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)
main.add_module('SVDDQMExpressReco', offlineZSShaperDigits='SVDShaperDigitsZS5')

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
