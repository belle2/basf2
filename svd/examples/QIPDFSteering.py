#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Steering file to generate cluster and spacePoint quality indicator pdf files.
# Please choose between 'cluster' and 'spacepoint' pdf generation scripts by running
# basf2 ./QIPDFSteering -- -i <script> with either string as the argument

import os
import argparse
from sys import argv
from basf2 import *
from time import time
from beamparameters import add_beamparameters

from spacePointQIPDFGen import spacePointQIPDFGen
from clusterQIPDFGen import clusterQIPDFGen

import glob
import ROOT

numEvents = 1000

parser = argparse.ArgumentParser(description='Choose which PDF gen script you wish to run.')
parser.add_argument("-i", "--genScript", default="spacepoint", type=str)

args = parser.parse_args()
print(args.genScript + ' ' + 'PDF generation script will be run.')

set_log_level(LogLevel.ERROR)

main = create_path()

beamparameters = add_beamparameters(main, "Y4S")

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)

eventinfoprinter = register_module('EventInfoPrinter')
main.add_module(eventinfoprinter)

progress = register_module('Progress')
main.add_module(progress)

evtgen = register_module('EvtGenInput')
evtgen.param('ParentParticle', 'Upsilon(4S)')
main.add_module(evtgen)

gearbox = register_module('Gearbox')
main.add_module(gearbox)

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticField', 'PXD', 'SVD'])
main.add_module(geometry)

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
main.add_module(g4sim)

backgroundFiles = glob.glob('/group/belle2/BGFile/OfficialBKG/15thCampaign/bgoverlay_phase3/*.root')
bkgoverlayInput = register_module('BGOverlayInput')
bkgoverlayInput.param('inputFileNames', backgroundFiles)
main.add_module(bkgoverlayInput)

svdDigitizer = register_module('SVDDigitizer')
main.add_module(svdDigitizer)

bkgoverlay = register_module('BGOverlayExecutor')
main.add_module(bkgoverlay)

shaper = register_module('SVDShaperDigitSorter')
main.add_module(shaper)
svdFitter = register_module('SVDCoGTimeEstimator')
main.add_module(svdFitter)
svdClusterizer = register_module('SVDSimpleClusterizer')
main.add_module(svdClusterizer)

if args.genScript == 'spacepoint':
    spacePointPDFGen = spacePointQIPDFGen()
    main.add_module(spacePointPDFGen)
if args.genScript == 'cluster':
    clusterPDFGen = clusterQIPDFGen()
    main.add_module(clusterPDFGen)

process(main)

print('Event Statistics :')
print(statistics)
