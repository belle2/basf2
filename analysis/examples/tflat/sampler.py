#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
import modularAnalysis as ma
from tflat.FlavorTagger import flavorTagger


def reconstructB2nunubar(path):
    """
    Defines the procedure to create a B0 list for the benchmark channel 'B0 -> nu_tau anti-nu_tau'
    """
    ma.fillParticleListFromMC('nu_tau:MC', '', path=path)
    ma.reconstructMCDecay('B0:sig -> nu_tau:MC anti-nu_tau:MC', '', writeOut=True, path=path)


def buildROE(path):
    """
    Creates the rest of event for the signal particle list.'
    """
    # perform MC matching (MC truth association).
    ma.matchMCTruth(list_name='B0:sig', path=path)

    # build the rest of the event associated to the B0
    ma.buildRestOfEvent(target_list_name='B0:sig', path=path)


if __name__ == '__main__':
    '''
    Samples a chunk of training data for TFlat
    '''
    b2.set_log_level(b2.LogLevel.ERROR)
    b2.conditions.prepend_globaltag('analysis_tools_light-2501-betelgeuse')
    path = b2.Path()

    ma.inputMdstList(environmentType="default", filelist='', path=path)
    reconstructB2nunubar(path)
    buildROE(path)

    flavorTagger(
        'B0:sig',
        mode='Sampler',
        working_dir='',
        uniqueIdentifier='standard_tflat',
        path=path)
    ma.summaryOfLists(particleLists=['B0:sig'], path=path)
    b2.process(path)
    print(b2.statistics)
