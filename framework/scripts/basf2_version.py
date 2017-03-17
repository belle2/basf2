#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

version = ''
try:
    from ROOT import Belle2
    import ROOT
    ROOT.gSystem.Load('libframework_io')
    ROOT.gROOT.LoadMacro(Belle2.FileSystem.findFile("include/framework/io/RootIOUtilities.h"))
    version = Belle2.RootIOUtilities.getCommitID()
except AttributeError:
    pass

release = (os.environ.get('BELLE2_RELEASE', 'unknown'))

if __name__ == "__main__":
    print('BASF2, Version: %s (Release: %s)' % (version, release))
