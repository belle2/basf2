#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# neutrals lists
from stdPhotons import *
from stdPi0s import *

# charged lists
from stdCharged import *

# Prepare all standard final state particles


def stdFSParticles(path=analysis_main):
    """
    Creation of default particle lists. Please find criterias for traks
    at `stdPi()`, `stdK()`, `stdPr()`, `stdPhotons()`, `stdPi0s()`, `stdKs()`

    >>stdFSParticles()
    """

    # Nominal PID (used to be >0.5 cut)
    stdPi('95eff', path)
    stdK('95eff', path)
    stdPr('95eff', path)
    # stdE(?) - missing
    # stdMu(?) - missing

    # standard photons
    stdPhotons('loose', path)

    # reconstruct standard pi0s including a mass constraint fit
    stdPi0s('veryLooseFit', path)

    # reconstruct standard Ks
    stdKs()

    # Egamma > 1.5 GeV
#    stdHighEPhoton(path)


def stdKs(path=analysis_main):
    """
    Creation of standrd 'K_S0:all' list from 'pi+:all' list (see description at `stdPi()`).
    Additional requirements to K_S0:
    '0.477614<M<0.517614'

    >>stdKs()
    """

    stdPi('all')
    reconstructDecay('K_S0:all -> pi-:all pi+:all', '0.4 < M < 0.6', 1, True,
                     path)
    vertexKFit('K_S0:all', 0.0)
    applyCuts('K_S0:all', '0.477614<M<0.517614')
    matchMCTruth('K_S0:all')
