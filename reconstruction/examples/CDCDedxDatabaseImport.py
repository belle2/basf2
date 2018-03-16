#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use the CDCDedxDatabaseImporter
# to import dummy data (from TH1) into a payload ROOT file
#
# The user should provide input and output root filname
# as first and second argument respectively.
#
# Example steering file - 2017 Belle II Collaboration
# Author(s): Jake Bennett
#
########################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import CDCDedxDatabaseImporter

dedxScaleImporter = CDCDedxDatabaseImporter("DedxPDFs.root", "DedxPDFs")
dedxScaleImporter.importPDFs()

dedxScaleImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxScaleFactor")
dedxScaleImporter.importScaleFactor(52.1)

dedxCurveImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxCurvePars")
dedxCurveImporter.importCurveParameters()

dedxSigmaImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxSigmaPars")
dedxSigmaImporter.importSigmaParameters()

dedxHadronImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxHadronCor")
dedxHadronImporter.importHadronCorrection()
