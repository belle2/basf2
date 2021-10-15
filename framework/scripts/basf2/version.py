#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import basf2


def get_version():
    version = ''
    try:
        from ROOT import Belle2
        import ROOT
        ROOT.gSystem.Load('libframework_io')
        ROOT.gROOT.LoadMacro(basf2.find_file("include/framework/io/RootIOUtilities.h"))
        version = Belle2.RootIOUtilities.getCommitID()
    except AttributeError:
        pass
    return version


def get_release():
    release = (os.environ.get('BELLE2_RELEASE', 'unknown'))
    return release


if __name__ == "__main__":
    print(f'basf2 (Belle II Analysis Software Framework), Version: {get_version()} (Release: {get_release()})')
