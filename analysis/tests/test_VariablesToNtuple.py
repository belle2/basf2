#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
import ROOT
from ROOT import Belle2

filepath = 'analysis/tests/mdst7.root'
inputFile = Belle2.FileSystem.findFile(filepath)
if len(inputFile) == 0:
    sys.stderr.write(
        "TEST SKIPPED: input file " +
        filepath +
        " not found. You can retrieve it via 'wget http://www-ekp.physik.uni-karlsruhe.de/~tkeck/mdst7.root'\n")
    sys.exit(-1)

path = create_path()
path.add_module('RootInput', inputFileName=inputFile)
path.add_module('Gearbox')
path.add_module('Geometry', components=['MagneticField'])
path.add_module('ParticleLoader', decayStringsWithCuts=[('e+', '')])

# Write out electron id and momentum of all true electron candidates and every 10th wrong electron candidate
path.add_module('VariablesToNtuple',
                particleList='e+',
                variables=['electronID', 'p', 'isSignal'],
                sampling=('isSignal', {1: 0, 0: 20}),
                fileName='particleListNtuple.root',
                treeName='particleListTree')

# Write out number of tracks and ecl-clusters in every event, except for events with 12 tracks where we take only every 100th events
path.add_module('VariablesToNtuple',
                particleList='',
                variables=['nTracks', 'nECLClusters'],
                sampling=('nTracks', {12: 10}),
                fileName='eventNtuple.root',
                treeName='eventTree')


with tempfile.TemporaryDirectory() as tempdir:
    os.chdir(tempdir)
    process(path)

    # Testing
    assert os.path.isfile('particleListNtuple.root'), "particleListNtuple.root wasn't created"
    f = ROOT.TFile('particleListNtuple.root')
    t = f.Get('particleListTree')
    assert bool(t), "particleListTree isn't contained in file"
    assert t.GetListOfBranches().Contains('electronID'), "electronID branch is missing"
    assert t.GetListOfBranches().Contains('p'), "electronID branch is missing"
    assert t.GetListOfBranches().Contains('__weight__'), "weight branch is missing"

    nSignal = 0
    nBckgrd = 0
    for event in t:
        if event.isSignal == 1:
            assert event.__weight__ == 1, "Expected weight 1 for a true candidate got {}".format(event.__weight__)
            nSignal += 1
        else:
            assert event.__weight__ == 20, "Expected weight 20 for a wrong candidate got {}".format(event.__weight__)
            nBckgrd += 1
    assert nBckgrd < nSignal, "Expected less background than signal due to the large sampling rate"

    assert os.path.isfile('eventNtuple.root'), "eventNtuple.root wasn't created"
    f = ROOT.TFile('eventNtuple.root')
    t = f.Get('eventTree')
    assert bool(t), "eventTree isn't contained in file"
    assert t.GetListOfBranches().Contains('nTracks'), "nTracks branch is missing"
    assert t.GetListOfBranches().Contains('nECLClusters'), "nECLClusters branch is missing"
    assert t.GetListOfBranches().Contains('__weight__'), "weight branch is missing"

    nTracks_12 = 0
    nTracks_11 = 0
    for event in t:
        if event.nTracks == 12:
            assert event.__weight__ == 10, "Expected weight 10 in an event with 12 tracks got {}".format(event.__weight__)
            nTracks_12 += 1
        else:
            assert event.__weight__ == 1, "Expected weight 1 in an event with unequal 12 tracks got {}".format(event.__weight__)
            if event.nTracks == 11:
                nTracks_11 += 1
    assert nTracks_12 * 5 < nTracks_11, "Expected much less events with 12 tracks than with 11, due to the large sampling rate"
