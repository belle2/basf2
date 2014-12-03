#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_output(
    path,
    bgType,
    realTime,
    sampleType,
    fileName='output.root',
    ):
    '''
    A function to be used for output of BG simulation.
    @param path path name
    @param bgType background type, to get available types: basf2 -m BeamBkgTagSetter
    @param realTime equivalent time of superKEKB running in [ns]
    @param sampleType 'study' (for BG studies) or 'usual', 'PXD', 'ECL' (for BG mixer)
    @param fileName optional file name, can be overridden by basf2 -o 
    '''

    if sampleType is 'study':
        madeFor = ''
        branches = []
    elif sampleType is 'usual':
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
    elif sampleType is 'ECL':
        madeFor = 'ECL'
        branches = ['ECLHits']
    elif sampleType is 'PXD':
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
    path.add_module(tagSetter)

    # Write out only non-empty events when producing samples for BG mixer
    if sampleType is not 'study':
        emptyPath = create_path()
        tagSetter.if_false(emptyPath)

    # Output to file
    output = register_module('RootOutput')
    output.param('outputFileName', fileName)
    output.param('branchNames', branches)
    path.add_module(output)


