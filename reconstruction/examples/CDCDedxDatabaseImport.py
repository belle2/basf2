#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

from ROOT.Belle2 import CDCDedxDatabaseImporter

dedxScaleImporter = CDCDedxDatabaseImporter("DedxPDFs.root", "DedxPDFs")
dedxScaleImporter.importPDFs()

dedxScaleImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxScaleFactor")
dedxScaleImporter.importScaleFactor(35.428)

dedxMeanImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxMeanPars")
dedxMeanImporter.importMeanParameters()

dedxSigmaImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxSigmaPars")
dedxSigmaImporter.importSigmaParameters()

dedxHadronImporter = CDCDedxDatabaseImporter("DedxPars.root", "CDCDedxHadronCor")
dedxHadronImporter.importHadronCorrection()
