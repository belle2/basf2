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
import pandas
import b2test_utils
from b2pandas_utils import VariablesToHDF5

inputFile = b2test_utils.require_file('mdst14.root', 'validation')
path = basf2.create_path()
path.add_module('RootInput', inputFileName=inputFile)
path.add_module('ParticleLoader', decayStrings=['e+'])

# Write out electron id and momentum of all true electron candidates
v2hdf5_e = VariablesToHDF5(
    "e+:all", ['electronID', 'p', 'isSignal'], "particleDF.hdf5")
path.add_module(v2hdf5_e)

# event-wise mode is not supported at the moment. when it is add something like
# the following comment and test file creation
# Write out number of tracks and ecl-clusters in every event
# v2hdf5_evt = VariablesToHDF5(
# "", ['nTracks', 'nKLMClusters'], "eventDF.hdf5")
# path.add_module(v2hdf5_evt)


with b2test_utils.clean_working_directory():
    basf2.process(path, 10)  # v2hdf5 is a python module, so don't run over everything... remove this if it gets implemented in C++

    # Testing
    assert os.path.isfile('particleDF.hdf5'), "particleDF.hdf5 wasn't created"
    df1 = pandas.read_hdf('particleDF.hdf5', 'e+:all')
    assert len(df1) > 0, "electron dataframe contains zero entries"
    assert 'electronID' in df1.columns, "electronID column is missing from electron dataframe"
    assert 'p' in df1.columns, "p column is missing from electron dataframe"
    assert 'evt' in df1.columns, "event number is missing from electron dataframe"
    assert 'run' in df1.columns, "run number is missing from electron dataframe"
    assert 'exp' in df1.columns, "experiment number is missing from electron dataframe"
    assert 'icand' in df1.columns, "candidate number is missing from electron dataframe"
    assert 'ncand' in df1.columns, "candidate count is missing from electron dataframe"

    assert df1.run[0] == 0, "run number not as expected"
    assert df1.exp[0] == 1003, "experiment number not as expected"
    assert df1.evt[0] == 1, "event number not as expected"
