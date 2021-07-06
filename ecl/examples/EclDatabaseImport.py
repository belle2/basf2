#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file shows how to use the EclDatabaseImport
   to import dummy data (as TH1) into a payload ROOT file.

The user should provide input and output root file name as
first and second argument, respectively. Comment in the
importer you need at the end. Upload the local DB afterwards.
"""

import sys
import glob
import basf2 as b2
import ROOT
from ROOT.Belle2 import ECLDatabaseImporter


if len(sys.argv) != 3:
    print('Usage: basf2 {} inputFile outputFile'.format(sys.argv[0]))
    sys.exit(1)

inputFile = sys.argv[1]
outputFile = sys.argv[2]

# Register 'EventInfoSetter' module
eventInfoSetter = b2.register_module('EventInfoSetter')
eventInfoSetter.initialize()

# Register 'Gearbox' module
gearbox = b2.register_module('Gearbox')
gearbox.initialize()

rootFiles = ROOT.vector('string')()
[rootFiles.push_back(name) for name in glob.glob(inputFile)]
eclDBImporter = ECLDatabaseImporter(rootFiles, outputFile)

"""Import ECL energy calibration constants to the database
(examples/CreateEclDigitTestPayload.C)
"""
eclDBImporter.importDigitEnergyCalibration()

"""Import ECL time calibration constants to the database
(input file from Chris Hearty)
"""
# eclDBImporter.importDigitTimeCalibration()

"""Import ECL shower shape corrections to second moment
to the database (input file from Savino Longo)
"""
# eclDBImporter.importShowerShapesSecondMomentCorrections()

"""Import ECL leakage corrections to showers
(input file from Suman Koirala)
"""
# eclDBImporter.importShowerCorrectorLeakageCorrections()

"""Import threshold values for track ECL cluster matching
(input file from Frank Meier)
"""
# eclDBImporter.importTrackClusterMatchingThresholds()

"""Import RMS parameterizations for track ECL cluster matching
(input file from Frank Meier)
"""
# eclDBImporter.importTrackClusterMatchingParameterizations()
