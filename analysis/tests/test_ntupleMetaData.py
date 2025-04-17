#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import socket
from ROOT import Belle2, TFile
import basf2 as b2
from basf2.version import get_version
from b2test_utils import safe_process, clean_working_directory

import os
import subprocess
import json
from datetime import datetime

b2.set_random_seed("something random")

testFile = 'ntuplemetadata.root'
inputFile1 = 'test1.root'
inputFile2 = 'test2.root'
dataDescription1 = {"adding_custom_key": "works"}
dataDescription2 = {"with_multiple": "trees"}


def get_metadata(name="output.root"):
    """Get the metadata out of a root file"""
    out = TFile(name)
    t = out.Get("persistent")
    t.GetEntry(0)
    return Belle2.FileMetaData(t.FileMetaData)


def generate_test_data(filename, evtNumList, runList, expList):
    main = b2.create_path()

    main.add_module("EventInfoSetter", evtNumList=evtNumList, runList=runList, expList=expList)
    main.add_module("RootOutput", outputFileName=filename)

    b2.process(main)

    return os.path.abspath(filename)


with clean_working_directory() as tmpdir:

    # generate input files
    evtNumList = [9, 1]
    runList = [1, 30]
    expList = [7, 7]
    generate_test_data(inputFile1, evtNumList, runList, expList)

    evtNumList = [19, 11]
    runList = [17, 19]
    expList = [9, 9]
    generate_test_data(inputFile2, evtNumList, runList, expList)

    # generate a small test ntuple output file
    main = b2.Path()
    main.add_module('RootInput', inputFileNames=[inputFile1, inputFile2])
    main.add_module('VariablesToNtuple',
                    fileName=testFile,
                    treeName='ntupleTree',
                    dataDescription=dataDescription1
                    )
    main.add_module('VariablesToNtuple',
                    fileName=testFile,
                    treeName='anotherNtupleTree',
                    dataDescription=dataDescription2
                    )
    safe_process(main)

    metadata = get_metadata(testFile)

    assert os.path.abspath(testFile) == tmpdir + '/' + metadata.getLfn()
    assert 40 == metadata.getNEvents()
    assert 40 == metadata.getNFullEvents()

    assert 7 == metadata.getExperimentLow()
    assert 1 == metadata.getRunLow()
    assert 1 == metadata.getEventLow()
    assert 9 == metadata.getExperimentHigh()
    assert 19 == metadata.getRunHigh()
    assert 11 == metadata.getEventHigh()  # highest evt. no in highest run
    assert metadata.containsEvent(7, 1, 1)
    assert metadata.containsEvent(7, 1, 9)
    assert metadata.containsEvent(7, 15, 1)
    assert metadata.containsEvent(7, 16, 6)
    assert metadata.containsEvent(9, 16, 12)
    assert metadata.containsEvent(9, 18, 19)
    assert metadata.containsEvent(9, 19, 11)
    assert not metadata.containsEvent(7, 0, 10)
    assert not metadata.containsEvent(10, 16, 6)
    assert not metadata.containsEvent(0, 15, 6)

    assert 2 == metadata.getNParents()
    assert os.path.abspath(inputFile1) == metadata.getParent(0)
    assert os.path.abspath(inputFile2) == metadata.getParent(1)

    assert datetime.today().strftime('%Y-%m-%d') == metadata.getDate()[:10]
    assert socket.gethostname() == metadata.getSite()

    assert "something random" == metadata.getRandomSeed()

    assert get_version() == metadata.getRelease()
    assert metadata.getSteering().startswith('#!/usr/bin/env python3')
    assert metadata.getSteering().strip().endswith('dummystring')
    assert 40 == metadata.getMcEvents()

    assert metadata.getDataDescription()["isNtupleMetaData"]
    assert "works" == metadata.getDataDescription()["adding_custom_key"]
    assert "trees" == metadata.getDataDescription()["with_multiple"]

    os.system('touch Belle2FileCatalog.xml')
    assert 0 == os.system('b2file-metadata-add --lfn /logical/file/name ' + testFile)

    assert 0 == os.system('b2file-metadata-show ' + testFile)

    # Check JSON output (contains steering file, so we cannot use .out)
    metadata_output = subprocess.check_output(['b2file-metadata-show', '--json', testFile])
    m = json.loads(metadata_output.decode('utf-8'))
    assert 7 == m['experimentLow']
    assert 1 == m['runLow']
    assert 1 == m['eventLow']
    assert 9 == m['experimentHigh']
    assert 19 == m['runHigh']
    assert 11 == m['eventHigh']
    assert 'something random' == m['randomSeed']
    assert 40 == m['nEvents']
    assert isinstance(m['nEvents'], int)
    assert 40 == m['nFullEvents']
    assert isinstance(m['nFullEvents'], int)
    assert '/logical/file/name' == m['LFN']

    # steering file is in metadata, so we check for existence of this string:
    # dummystring
