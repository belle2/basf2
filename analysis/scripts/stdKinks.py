#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma


def stdKKinks(path=None, writeOut=False, daughter='pi+'):
    """
    Load a list of kaon kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).
    Kaon to muon and kaon to pion decays are hard to distinguish, so they are both saved by default.

    The ParticleList is named ``K+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinematic cuts to suppress pion decays :math:`pPimu > 0.05~GeV` and
    muon decays :math:`pPemu > 0.07~GeV` are applied. In addition, events with wrong kinematics are discarded
    :math:`pKpi < 0.3~GeV`.

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
        daughter (str): the daughter to use in the decay string
    """
    # Fill list from Kinks
    decayString = 'K+:kinks -> ' + daughter
    ma.fillParticleList(decayString, '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('K+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts(
        'K+:kinks',
        'kinkDaughterMomentumInMotherRestFramePiMuHypothesis > 0.05 and \
         kinkDaughterMomentumInMotherRestFrameMuEHypothesis > 0.07 and \
         kinkDaughterMomentumInMotherRestFrameKPiHypothesis < 0.3 and \
         kinkFilterID < 3',
        path=path)


def stdPiKinks(path=None, writeOut=False, daughter='mu+'):
    """
    Load a list of pion kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).

    The ParticleList is named ``pi+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinematic cut to suppress other decays is applied :math:`pPimu < 0.1~GeV`.

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
        daughter (str): the daughter to use in the decay string
    """
    # Fill list from Kinks
    decayString = 'pi+:kinks -> ' + daughter
    ma.fillParticleList(decayString, '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('pi+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts('pi+:kinks', 'kinkDaughterMomentumInMotherRestFramePiMuHypothesis < 0.1 and kinkFilterID < 3', path=path)


def stdMuKinks(path=None, writeOut=False, daughter='e+'):
    """
    Load a list of muon kinks from Kink objects using only kinks created from track pairs (Filter 1 and 2).

    The ParticleList is named ``mu+:kinks``. A simple clone suppression is applied based on the
    combined fit result flag. Loose kinematic cuts to suppress pion decays :math:`pPimu > 0.05~GeV` and
    kaon decays :math:`pMue < 0.1~GeV` are applied.

    Parameters:
        path (basf2.Path): the path to load the modules
        writeOut (bool): whether RootOutput module should save the created ParticleList
        daughter (str): the daughter to use in the decay string
    """
    # Fill list from Kinks
    decayString = 'mu+:kinks -> ' + daughter
    ma.fillParticleList(decayString, '', writeOut=writeOut, path=path)
    clonesCut = ('kinkCombinedFitResultFlag > 15 or kinkCombinedFitResultFlagB1==0 and '
                 'kinkCombinedFitResultFlagB2==0 and kinkCombinedFitResultFlagB4==0')
    ma.applyCuts('mu+:kinks',
                 clonesCut,
                 path=path)
    ma.applyCuts(
        'mu+:kinks',
        'kinkDaughterMomentumInMotherRestFramePiMuHypothesis > 0.05 and \
         kinkDaughterMomentumInMotherRestFrameMuEHypothesis < 0.1 and kinkFilterID < 3',
        path=path)
