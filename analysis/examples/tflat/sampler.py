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


def reconstructB2jpsiks(path, is_belle=False):
    """
    Defines the procedure to create a B0 list for the benchmark channel 'B0 -> Jpsi KS'
    """
    ma.fillParticleList('mu+:all', cut='', path=path)
    ma.reconstructDecay('J/psi:mumu -> mu+:all mu-:all', cut='abs(dM) < 0.11', path=path)
    ma.matchMCTruth('J/psi:mumu', path=path)

    if is_belle:
        ma.matchMCTruth('K_S0:mdst', path=path)
        ma.reconstructDecay('B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.25 and abs(deltaE) < 0.15', path=path)
    else:
        ma.fillParticleList('pi+:all', cut='', path=path)
        ma.reconstructDecay('K_S0:pipi -> pi+:all pi-:all', cut='abs(dM) < 0.25', path=path)
        ma.matchMCTruth('K_S0:pipi', path=path)
        ma.reconstructDecay('B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.25 and abs(deltaE) < 0.15', path=path)


def buildROE(path):
    """
    Creates the rest of event for the signal particle list.'
    """
    # perform MC matching (MC truth association).
    ma.matchMCTruth(list_name='B0:sig', path=path)

    # keep only signal
    ma.applyCuts(list_name='B0:sig', cut='isSignal == 1', path=path)

    # build the rest of the event associated to the B0
    ma.buildRestOfEvent(target_list_name='B0:sig', path=path)


def main(uniqueIdentifier, inputfile='', working_dir='', is_belle=False, sampler_id=0, channel='nunu'):
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

        # Load and convert input files
        convertBelleMdstToBelleIIMdst(
            inputBelleMDSTFile=inputfile,
            applySkim=False,
            enableLocalDB=False,
            generatorLevelMCMatching=False,
            path=path)

        ma.setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path=path)
        b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltagB2BII())

    if channel == 'nunu':
        reconstructB2nunubar(path)
    elif channel == 'jpsiks':
        reconstructB2jpsiks(path, is_belle)
    else:
        raise ValueError("Unknown sampler channel!")

    buildROE(path)

    flavorTagger(
        'B0:sig',
        mode='Sampler',
        working_dir=working_dir,
        uniqueIdentifier=uniqueIdentifier,
        sampler_id=sampler_id,
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
    parser.add_argument(
        '--BELLE',
        metavar='BELLE',
        dest='BELLE',
        type=bool,
        default=False,
        help='Indicate if Belle or Belle II files are to be processed'
    )
    parser.add_argument(
        '--sampler_id',
        metavar='sampler_id',
        dest='sampler_id',
        type=int,
        default=0,
        help='sampler_id'
    )
    parser.add_argument(
        '--channel',
        metavar='channel',
        dest='channel',
        type=str,
        default='nunu',
        help='Sampler channel: nunu or jpsiks'
    )

    args = parser.parse_args()
    uniqueIdentifier = args.uniqueIdentifier
    inputfile = args.inputfile
    working_dir = args.working_dir
    is_belle = args.BELLE
    sampler_id = args.sampler_id
    channel = args.channel

    main(uniqueIdentifier, inputfile, working_dir, is_belle, sampler_id, channel)
