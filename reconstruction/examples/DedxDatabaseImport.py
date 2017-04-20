#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use the DedxDatabaseImport
# to import dummy data (as TH1) into a payload ROOT file
#
# The user should provide input and output root filname
# as first and second argument respectively.
#
# Example steering file - 2015 Belle II Collaboration
# Author(s): Jake Bennett
#
########################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import DedxDatabaseImporter
import glob
import sys

inputFile = sys.argv[1]
outputFile = sys.argv[2]

rootFiles = ROOT.vector('string')()
[rootFiles.push_back(name) for name in glob.glob(inputFile)]
dedxDBImporter = DedxDatabaseImporter(rootFiles, outputFile)
dedxDBImporter.importWireGainCalibration()
