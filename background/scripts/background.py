#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import create_path, B2ERROR
import os
import glob


def get_background_files(folder=None):
    """ Loads the location of the background files from the environmant variable
     BELLE2_BACKGROUND_DIR which is set on the validation server and ensures that background
     files exist and returns the list of background files which
     can be directly used with add_simulation() :

     add_simulation(main, bkgfiles=background.get_background_files())

     Will fail with an assert if no background folder set or if no background file was
     found in the set folder.

     @param A specific folder to search for background files can be given as an optional parameter
    """

    env_name = 'BELLE2_BACKGROUND_DIR'
    bg = None

    if folder is None:
        if env_name not in os.environ:
            print("Environment variable {} for backgound files not set. Terminanting this script.".format(env_name))
            assert False
        folder = os.environ[env_name]

    bg = glob.glob(folder + '/*.root')

    if len(bg) == 0:
        print("No background files found in folder {} . Terminating this script.".format(folder))
        assert False

    print("Background files loaded from folder {}".format(folder))

    # sort for easier comparison
    bg = sorted(bg)

    print("{: >65} {: >65} ".format("- Background file name -", "- file size -"))
    for f in bg:
        fsize = os.path.getsize(f)
        print("{: >65} {: >65} ".format(f, fsize))

    return bg


def add_output(path, bgType, realTime, sampleType, phase=3, fileName='output.root'):
    '''
    A function to be used for output of BG simulation.
    @param path path name
    @param bgType background type, to get available types: basf2 -m BeamBkgTagSetter
    @param realTime equivalent time of superKEKB running in [ns]
    @param sampleType 'study' (for BG studies) or 'usual', 'PXD', 'ECL' (for BG mixer)
    @param specify the Phase, 1 for Phase 1, 2 for Phase 2, and 3 for Physics Run or Phase 3
    @param fileName optional file name, can be overridden by basf2 -o
    '''

    if sampleType == 'study':
        madeFor = ''
        branches = []
    elif sampleType == 'usual' and phase == 3:
        madeFor = ''
        branches = [
            'PXDSimHits',
            'SVDSimHits',
            'CDCSimHits',
            'TOPSimHits',
            'ARICHSimHits',
            'ECLHits',
            'BKLMSimHits',
            'EKLMSimHits',
        ]
    elif sampleType == 'usual' and phase == 2:
        madeFor = ''
        branches = [
            'PXDSimHits',
            'SVDSimHits',
            'CDCSimHits',
            'TOPSimHits',
            'ARICHSimHits',
            'ECLHits',
            'BKLMSimHits',
            'EKLMSimHits',
            'CLAWSSimHits',
            'FANGSSimHits',
            'PlumeSimHits',
            'BeamabortSimHits',
            'PindiodeSimHits',
            'QcsmonitorSimHits',
            'He3tubeSimHits',
            'MicrotpcSimHits',
        ]
    elif sampleType == 'usual' and phase == 1:
        madeFor = ''
        branches = [
            'ClawSimHits',
            'BeamabortSimHits',
            'PindiodeSimHits',
            'QcsmonitorSimHits',
            'He3tubeSimHits',
            'MicrotpcSimHits',
            'BgoSimHits',
            'CsiSimHits',
        ]
    elif sampleType == 'ECL':
        madeFor = 'ECL'
        branches = ['ECLHits']
    elif sampleType == 'PXD':
        madeFor = 'PXD'
        branches = ['PXDSimHits']
    else:
        madeFor = ''
        branches = []
        B2ERROR('add_output - invalid value of argument sampleType: %s'
                % sampleType)

    # Set background tag in SimHits and add BackgroundMetaData into persistent tree
    tagSetter = path.add_module('BeamBkgTagSetter', backgroundType=bgType, realTime=realTime,
                                specialFor=madeFor, Phase=phase)

    # Write out only non-empty events when producing samples for BG mixer
    if sampleType != 'study':
        emptyPath = create_path()
        tagSetter.if_false(emptyPath)

    # Output to file. We don't need a TTreeIndex for background files and memory
    # consumption can be improved by setting a lower autoFlushSize so that
    # fewer and or smaller amounts of data have to be read for each GetEntry()
    path.add_module('RootOutput', outputFileName=fileName, branchNames=branches,
                    buildIndex=False, autoFlushSize=-500000)
