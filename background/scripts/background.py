#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_output(path, bgType, realTime, sampleType, phase=3, fileName='output.root'):
    '''
    A function to be used for output of BG simulation.
    @param path path name
    @param specify the Phase, 1 for Phase 1, 2 for Phase 2, and 3 for Physics Run or Phase 3
    @param bgType background type, to get available types: basf2 -m BeamBkgTagSetter
    @param realTime equivalent time of superKEKB running in [ns]
    @param sampleType 'study' (for BG studies) or 'usual', 'PXD', 'ECL' (for BG mixer)
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
    tagSetter = register_module('BeamBkgTagSetter')
    tagSetter.param('backgroundType', bgType)
    tagSetter.param('realTime', realTime)
    tagSetter.param('specialFor', madeFor)
    tagSetter.param('Phase', phase)
    path.add_module(tagSetter)

    # Write out only non-empty events when producing samples for BG mixer
    if sampleType != 'study':
        emptyPath = create_path()
        tagSetter.if_false(emptyPath)

    # Output to file
    output = register_module('RootOutput')
    output.param('outputFileName', fileName)
    output.param('branchNames', branches)
    path.add_module(output)
