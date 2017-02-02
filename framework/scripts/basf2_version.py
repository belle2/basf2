#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
import ROOT

from ROOT import Belle2
ROOT.gSystem.Load('libframework_io')
ROOT.gROOT.LoadMacro(Belle2.FileSystem.findFile("include/framework/io/RootIOUtilities.h"))
try:
    version = Belle2.RootIOUtilities.getCommitID()
except AttributeError:
    version = os.environ.get('BELLE2_RELEASE', 'unknown')

if __name__ == "__main__":
    print('BASF2, Version', version)
