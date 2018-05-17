#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################################
#
# This steering file shows how to use the ECLEoPDatabaseImporter
# to import dummy data (from .dat) into a payload ROOT file
#
# The user should provide input (.dat) and output (ROOT) filname
# as first and second argument respectively.
#
# Example steering file - 2018 Belle II Collaboration
# Author(s): Marco Milesi (marco.milesi@unimelb.edu.au)
#
########################################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import ECLEoPDatabaseImporter

e_eopDBImporter = ECLEoPDatabaseImporter("electrons_N1.dat", "e_ECLEoPPDF")
e_eopDBImporter.importPDF()
