#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma


def stdKKinks(path=None, writeOut=False):
    """
    Load a list of the Kaon kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).
    Kaon to muon and kaon to pion decays hard to distinguish, so they are both saved.

    The ParticleList is named ``K+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinemtic cuts to suppress pion decays :math:`pPimu > 0.05~GeV` and
    muon decays :math:`pPemu > 0.07~GeV` are applied. In addition, events with wrong kinematics are discarded
    :math:`pKpi < 0.3~GeV`.

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
    """
    # Fill list from Kinks
    ma.fillParticleList('K+:kinks -> pi+', '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('K+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts('K+:kinks',
                 'pPimu > 0.05 and pMue > 0.07 and pKpi < 0.3 and kinkFilterID < 3',
                 path=path)


def stdPiKinks(path=None, writeOut=False):
    """
    Load a list of the Pion kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).

    The ParticleList is named ``pi+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinemtic cut to suppress other decays is applied :math:`pPimu < 0.1~GeV`.

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
    """
    # Fill list from Kinks
    ma.fillParticleList('pi+:kinks -> mu+', '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('pi+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts('pi+:kinks', 'pPimu < 0.1 and kinkFilterID < 3', path=path)


def stdMuKinks(path=None, writeOut=False):
    """
    Load a list of the Muon kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).

    The ParticleList is named ``mu+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinemtic cuts to suppress pion decays :math:`pPimu > 0.05~GeV` and
    kaon decays :math:`pMue < 0.1~GeV` are applied/

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
    """
    # Fill list from Kinks
    ma.fillParticleList('mu+:kinks -> e+', '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('mu+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts('mu+:kinks', 'pPimu > 0.05 and pMue < 0.1 and kinkFilterID < 3', path=path)
