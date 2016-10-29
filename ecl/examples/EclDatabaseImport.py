#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use the EclDatabaseImport
# to import dummy data (as TH1) into a payload ROOT file
#
# The user should provide input and output root filname
# as first and second argument respectively. Comment
# in the importer you need at the end.Upload the local DB
# afterwards.
#
# Example steering file - 2015 Belle II Collaboration
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#            Alon Hershenhorn (hersehn@physics.ubc.ca)
#
########################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import ECLDatabaseImporter
import glob
import sys

inputFile = sys.argv[1]
outputFile = sys.argv[2]

# register event info setter
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

# register gearbox
gearbox = register_module('Gearbox')
gearbox.initialize()

rootFiles = ROOT.vector('string')()
[rootFiles.push_back(name) for name in glob.glob(inputFile)]
eclDBImporter = ECLDatabaseImporter(rootFiles, outputFile)

# Import ECL energy calibration constants to the database (examples/CreateEclDigitTestPayload.C)
eclDBImporter.importDigitEnergyCalibration()

# Import ECL time calibration constants to the database (input file from Chris Hearty)
# eclDBImporter.importDigitTimeCalibration()

# Import ECL shower shape corrections to second moment to the database (input file from Savino Longo)
# eclDBImporter.importShowerShapesSecondMomentCorrections()

# Import ECL leakage corrections to showers (input file from Suman Koirala)
# eclDBImporter.importShowerCorrectorLeakageCorrections()
