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
import ROOT
import b2test_utils

inputFile = b2test_utils.require_file('mdst14.root', 'validation')
path = basf2.create_path()
path.add_module('RootInput', inputFileName=inputFile)
path.add_module('ParticleLoader', decayStrings=['e+'])

# Write out electron id and mc information of all electron candidates into histograms
path.add_module('VariablesToHistogram',
                particleList='e+:all',
                variables=[('electronID', 100, 0, 1), ('isSignal', 2, -0.5, 1.5)],
                variables_2d=[('mcErrors', 513, -0.5, 512.5, 'isSignal', 2, -0.5, 1.5)],
                fileName='particleListNtuple.root')

# Write out number of tracks and ecl-clusters in every event into histograms
path.add_module('VariablesToHistogram',
                particleList='',
                variables=[('nTracks', 31, -0.5, 30.5), ('nKLMClusters', 51, -0.5, 50.5)],
                variables_2d=[('nTracks', 31, -0.5, 30.5, 'nKLMClusters', 51, -0.5, 50.5)],
                fileName='eventNtuple.root')


with b2test_utils.clean_working_directory():
    basf2.process(path)

    # Testing
    assert os.path.isfile('particleListNtuple.root'), "particleListNtuple.root wasn't created"
    f = ROOT.TFile('particleListNtuple.root')
    t = f.Get('electronID')
    assert bool(t), "electronID histogram isn't contained in file"
    assert t.GetBinContent(1) > t.GetBinContent(
        100), 'Expected fewer candidates with a highest electronID compared to lowest electronID'
    t = f.Get('isSignal')
    assert bool(t), "isSignal histogram isn't contained in file"
    assert t.GetBinContent(1) > t.GetBinContent(2), 'Expected more background than signal'
    signal_1d = t.GetBinContent(2)
    t = f.Get('mcErrorsisSignal')
    assert bool(t), "mcErrorsisSignal histogram isn't contained in file"

    signal_2d = []
    for i in range(513):
        if i >= 4:
            assert t.GetBinContent(i, 2) == 0, 'Expected no entries for mcError >= 4 and true electron candidates'
        else:
            assert t.GetBinContent(i, 1) == 0, 'Expected no entries for mcError < 4 and false electron candidates'
            signal_2d.append(t.GetBinContent(i, 2))
    assert signal_1d == sum(signal_2d), f'Expected same amount of signal in 1D and 2D histograms got {signal_1d} {sum(signal_2d)}'

    assert os.path.isfile('eventNtuple.root'), "eventNtuple.root wasn't created"
    f = ROOT.TFile('eventNtuple.root')
    t = f.Get('nTracks')
    assert bool(t), "nTracks histogram isn't contained in file"
    assert t.GetBinContent(13) > t.GetBinContent(1), 'Expected more 12 track events than 1 track events'
    assert t.GetBinContent(13) > t.GetBinContent(2), 'Expected more 12 track events than 2 track events'
    assert t.GetBinContent(13) > t.GetBinContent(3), 'Expected more 12 track events than 3 track events'
    assert t.GetBinContent(13) > t.GetBinContent(4), 'Expected more 12 track events than 4 track events'
    assert t.GetBinContent(13) > t.GetBinContent(26), 'Expected more 12 track events than 26 track events'
    assert t.GetBinContent(13) > t.GetBinContent(27), 'Expected more 12 track events than 27 track events'
    assert t.GetBinContent(13) > t.GetBinContent(28), 'Expected more 12 track events than 28 track events'
    assert t.GetBinContent(13) > t.GetBinContent(29), 'Expected more 12 track events than 29 track events'
    ntracks_12_1d = t.GetBinContent(13)

    t = f.Get('nKLMClusters')
    assert bool(t), "nKLMClusters histogram isn't contained in file"

    t = f.Get('nTracksnKLMClusters')
    assert bool(t), "nTracksnKLMClusters 2d histogram isn't contained in file"
    ntracks_12_2d = []
    for i in range(53):
        ntracks_12_2d.append(t.GetBinContent(13, i))
    assert ntracks_12_1d == sum(ntracks_12_2d),\
        f'Expected same amount of events with 12 tracks in 1D and 2D histograms got {ntracks_12_1d} {sum(ntracks_12_2d)}'
