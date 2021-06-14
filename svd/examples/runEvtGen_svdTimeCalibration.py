#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 1000 BBbar events are generated using the BabayagaNLO
# generator
#
# Example steering file
########################################################

import os
import basf2
import ROOT
from ROOT import Belle2
from basf2 import *
from basf2 import conditions as b2conditions
from tracking import add_tracking_reconstruction  # as trk
import rawdata as raw
from ROOT import Belle2
import os.path
import sys

import simulation as simu
import modularAnalysis as ana

import re
import glob

outputfilename = sys.argv[1]
# Set the global log level
# basf2.set_log_level(basf2.LogLevel.INFO)
basf2.set_log_level(basf2.LogLevel.ERROR)

b2conditions.globaltags = [
    'svd_NOCoGCorrections'
]

# main path
main = basf2.create_path()

# event info setter
runnum = int(sys.argv[2])
# main.add_module("EventInfoSetter", expList=1003, runList=1, evtNumList=1000)
# main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000)
main.add_module("EventInfoSetter", expList=0, runList=runnum, evtNumList=1000)

# generate BBbar events
main.add_module('EvtGenInput')

# simulate
bg = [fn for fn in
      # glob.glob('/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root')
      # glob.glob('/group/belle2/BGFile/OfficialBKG/early_phase3/5a1f0a9f2ad84a/overlay/phase31/BGx1/set0/*.root')
      glob.glob('/group/belle2/BGFile/OfficialBKG/nominal_phase3/prerelease-04-00-00a/mixing/phase3/set0/' + '/*.root')
      # glob.glob('/group/belle2/BGFile/OfficialBKG/nominal_phase3/5a1f0a9f2ad84a/overlay/phase3/BGx1/set0/*.root')
      if not re.search('.*ECL.*\\.root$', os.path.basename(fn))]
simu.add_simulation(main,
                    simulateT0jitter=True,
                    #                    bkgfiles=bg,
                    bkgOverlay=False,
                    usePXDDataReduction=False,
                    )
# scalef = 1
# set_module_parameters(main, type="BeamBkgMixer", overallScaleFactor=scalef)

# tracking reconstruction
# add_tracking_reconstruction(main)
add_tracking_reconstruction(main, components=['PXD', 'SVD', 'CDC'])  # TOP and ECL may be needed if used for EventT0 estimation
main.add_module("SVDShaperDigitsFromTracks")

# Create the main path and add the modules
main.add_module("Progress")
main.add_module("RootOutput", outputFileName=outputfilename,
                branchNames=['SVDShaperDigits', 'SVDShaperDigitsFromTracks', 'EventT0', 'SVDEventInfoSim', 'Tracks', 'RecoTracks']
                )

# main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# you can change the latency here
# newSS = 9.06
# print("Changing the latency in the digitizer to " + str(newSS))
# basf2.set_module_parameters(main, "SVDDigitizer", StartSampling=float(newSS))

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
