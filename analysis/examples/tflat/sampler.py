#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import argparse
import basf2 as b2
import modularAnalysis as ma
from tflat.flavorTagger import flavorTagger
from b2biiConversion import convertBelleMdstToBelleIIMdst
import os


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


def main(uniqueIdentifier, inputfile='', working_dir='', is_belle=False):
    '''
    Samples a chunk of training data for TFlat
    '''

    b2.set_log_level(b2.LogLevel.ERROR)
    path = b2.Path()

    if not is_belle:
        b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())
        ma.inputMdstList(environmentType="default", filelist='', path=path)
    else:
        # Set Belle enviroment
        os.environ['USE_GRAND_REPROCESS_DATA'] = '1'
        os.environ['PGUSER'] = 'g0db'
        # environmentType = "Belle"

        # Load and convert input files
        convertBelleMdstToBelleIIMdst(
            inputBelleMDSTFile=inputfile,
            enableLocalDB=False,
            generatorLevelMCMatching=False,
            path=path)

        ma.setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path=path)
        b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltagB2BII())

    reconstructB2nunubar(path)
    buildROE(path)

    flavorTagger(
        'B0:sig',
        mode='Sampler',
        working_dir=working_dir,
        uniqueIdentifier=uniqueIdentifier,
        path=path)
    ma.summaryOfLists(particleLists=['B0:sig'], path=path)
    b2.process(path)
    print(b2.statistics)


if __name__ == '__main__':
    '''
    Samples a chunk of training data for TFlat
    '''
    parser = argparse.ArgumentParser(description='Sample TFlat')
    parser.add_argument(
        '--uniqueIdentifier',
        metavar='uniqueIdentifier',
        dest='uniqueIdentifier',
        type=str,
        default="TFlaT_MC16rd_light_2601_hyperion",
        help='Name of both the config .yaml to be used and the produced weightfile'
    )
    parser.add_argument(
        '--inputfile',
        metavar='inputfile',
        dest='inputfile',
        type=str,
        default='',
        help='Inputfile to sample'
    )
    parser.add_argument(
        '--working_dir',
        metavar='working_dir',
        dest='working_dir',
        type=str,
        default='',
        help='working_dir'
    )

    args = parser.parse_args()
    uniqueIdentifier = args.uniqueIdentifier
    inputfile = args.inputfile
    working_dir = args.working_dir

    main(uniqueIdentifier, inputfile, working_dir, is_belle=False)
