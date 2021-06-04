#!/usr/bin/env python3

import os
import basf2

version = ''
try:
    from ROOT import Belle2
    import ROOT
    ROOT.gSystem.Load('libframework_io')
    ROOT.gROOT.LoadMacro(basf2.find_file("include/framework/io/RootIOUtilities.h"))
    version = Belle2.RootIOUtilities.getCommitID()
except AttributeError:
    pass

release = (os.environ.get('BELLE2_RELEASE', 'unknown'))

if __name__ == "__main__":
    print(f'BASF2, Version: {version} (Release: {release})')
