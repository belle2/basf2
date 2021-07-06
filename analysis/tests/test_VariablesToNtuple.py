#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
path.add_module('ParticleLoader', decayStrings=['gamma'])
path.add_module('ParticleListManipulator', outputListName='gamma', inputListNames=['gamma:all'], cut='clusterE > 2.5')

# Write out electron id and momentum of all true electron candidates and every 10th wrong electron candidate
path.add_module('VariablesToNtuple',
                particleList='e+:all',
                variables=['electronID', 'p', 'isSignal'],
                sampling=('isSignal', {1: 0, 0: 20}),
                fileName='particleListNtuple.root',
                treeName='electronListTree')

# Write out two V2NT trees to the same file
path.add_module('VariablesToNtuple',
                particleList='gamma',
                variables=['clusterE', 'p', 'isSignal'],
                fileName='particleListNtuple.root',  # as above
                treeName='photonListTree')

# Write out number of tracks and ecl-clusters in every event, except for events with 12 tracks where we take only every 100th events
path.add_module('VariablesToNtuple',
                particleList='',
                variables=['nTracks', 'nKLMClusters'],
                sampling=('nTracks', {12: 10}),
                fileName='eventNtuple.root',
                treeName='eventTree')

# try to write out candidate counters even though they're already there
path.add_module('VariablesToNtuple',
                particleList='',
                variables=['expNum', 'runNum', 'evtNum'],
                fileName='countersNtuple.root',
                treeName='countersTree')


with b2test_utils.clean_working_directory():
    basf2.process(path)

    # Testing
    assert os.path.isfile('particleListNtuple.root'), "particleListNtuple.root wasn't created"
    f = ROOT.TFile('particleListNtuple.root')
    t1 = f.Get('electronListTree')
    t2 = f.Get('photonListTree')
    assert bool(t1), "electronListTree isn't contained in file"
    assert bool(t2), "photonListTree isn't contained in file"
    assert t1.GetEntries() > 0, "electronListTree contains zero entries"
    assert t2.GetEntries() > 0, "photonListTree contains zero entries"
    assert t1.GetListOfBranches().Contains('electronID'), "electronID branch is missing from electronListTree"
    assert t1.GetListOfBranches().Contains('p'), "p branch is missing from electronListTree"
    assert t1.GetListOfBranches().Contains('__weight__'), "weight branch is missing from electronListTree"
    assert t1.GetListOfBranches().Contains('__event__'), "event number branch is missing from electronList tree"
    assert t1.GetListOfBranches().Contains('__run__'), "run number branch is missing from electronList tree"
    assert t1.GetListOfBranches().Contains('__experiment__'), "experiment number branch is missing from electronList tree"
    assert t1.GetListOfBranches().Contains('__production__'), "production number branch is missing from electronList tree"
    assert t1.GetListOfBranches().Contains('__candidate__'), "candidate number branch is missing from electronList tree"
    assert t1.GetListOfBranches().Contains('__ncandidates__'), "candidate count branch is missing from electronList tree"

    assert t2.GetListOfBranches().Contains('clusterE'), "clusterEnergy branch is missing from photonListTree"
    assert t2.GetListOfBranches().Contains('p'), "p branch is missing from photonListTree"
    assert t2.GetListOfBranches().Contains('__weight__'), "weight branch is missing from photonListTree"
    assert t2.GetListOfBranches().Contains('__event__'), "event number branch is missing from photonList tree"
    assert t2.GetListOfBranches().Contains('__run__'), "run number branch is missing from photonList tree"
    assert t2.GetListOfBranches().Contains('__experiment__'), "experiment number branch is missing from photonList tree"
    assert t2.GetListOfBranches().Contains('__production__'), "production number branch is missing from photonList tree"
    assert t2.GetListOfBranches().Contains('__candidate__'), "candidate number branch is missing from photonList tree"
    assert t2.GetListOfBranches().Contains('__ncandidates__'), "candidate count branch is missing from photonList tree"

    nSignal = 0
    nBckgrd = 0
    for event in t1:
        if event.isSignal == 1:
            assert event.__weight__ == 1, f"Expected weight 1 for a true electron candidate got {event.__weight__}"
            nSignal += 1
        else:
            assert event.__weight__ == 20, f"Expected weight 20 for a wrong electron candidate got {event.__weight__}"
            nBckgrd += 1
    assert nBckgrd < nSignal, "Expected less background than signal due to the large sampling rate"

    for event in t2:
        assert event.__weight__ == 1, f"Expected weight 1 for all photon candidates got {event.__weight__}"

    assert os.path.isfile('eventNtuple.root'), "eventNtuple.root wasn't created"
    f = ROOT.TFile('eventNtuple.root')
    t = f.Get('eventTree')
    assert bool(t), "eventTree isn't contained in file"
    assert t.GetListOfBranches().Contains('nTracks'), "nTracks branch is missing"
    assert t.GetListOfBranches().Contains('nKLMClusters'), "nKLMClusters branch is missing"
    assert t.GetListOfBranches().Contains('__weight__'), "weight branch is missing"
    assert t.GetListOfBranches().Contains('__event__'), "event number branch is missing"
    assert t.GetListOfBranches().Contains('__run__'), "run number branch is missing"
    assert t.GetListOfBranches().Contains('__experiment__'), "experiment number branch is missing"
    assert t.GetListOfBranches().Contains('__production__'), "production number branch is missing"
    assert not t.GetListOfBranches().Contains('__candidate__'), "candidate number branch is present in eventwise tree"
    assert not t.GetListOfBranches().Contains('__ncandidates__'), "candidate count branch is present in eventwise tree"

    t.GetEntry(0)
    assert t.__run__ == 0, "run number not as expected"
    assert t.__experiment__ == 1003, "experiment number not as expected"
    assert t.__event__ == 1, "event number not as expected"
    assert t.__production__ == 0, "production number not as expected"

    nTracks_12 = 0
    nTracks_11 = 0
    for event in t:
        if event.nTracks == 12:
            assert event.__weight__ == 10, f"Expected weight 10 in an event with 12 tracks got {event.__weight__}"
            nTracks_12 += 1
        else:
            assert event.__weight__ == 1, f"Expected weight 1 in an event with unequal 12 tracks got {event.__weight__}"
            if event.nTracks == 11:
                nTracks_11 += 1
    assert nTracks_12 * 5 < nTracks_11, "Expected much less events with 12 tracks than with 11, due to the large sampling rate"

    assert os.path.isfile('countersNtuple.root'), "eventNtuple.root wasn't created"
    f = ROOT.TFile('countersNtuple.root')
    t = f.Get('countersTree')
    assert bool(t), "countersTree isn't contained in file"
    assert t.GetListOfBranches().Contains('__event__'), "event number branch is missing"
    assert t.GetListOfBranches().Contains('__run__'), "run number branch is missing"
    assert t.GetListOfBranches().Contains('__experiment__'), "experiment number branch is missing"

    t.GetEntry(0)
    assert t.__run__ == 0, "run number not as expected"
    assert t.__experiment__ == 1003, "experiment number not as expected"
    assert t.__event__ == 1, "event number not as expected"
    assert t.__production__ == 0, "production number not as expected"

    t.GetEntry(9)
    assert t.__run__ == 0, "run number not as expected"
    assert t.__experiment__ == 1003, "experiment number not as expected"
    assert t.__event__ == 10, "event number not as expected"
    assert t.__production__ == 0, "production number not as expected"
