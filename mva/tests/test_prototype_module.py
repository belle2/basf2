#!/usr/bin/env python3
# Thomas Keck 2016

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from b2test_utils import skip_test_if_light
skip_test_if_light()  # light builds don't contain generators; skip before trying to import  # noqa

import basf2 as b2

import modularAnalysis as ma
from generators import add_evtgen_generator

import tempfile
import os

from ROOT import Belle2

if __name__ == "__main__":

    path = b2.create_path()
    ma.setupEventInfo(100, path)
    add_evtgen_generator(path, 'signal', Belle2.FileSystem.findFile('analysis/examples/simulations/B2A101-Y4SEventGeneration.dec'))
    path.add_module('MVAPrototype', identifier='fake.xml')

    with tempfile.TemporaryDirectory() as tempdir:
        os.chdir(tempdir)

        # Create a fake weightfile
        content = """
            <?xml version="1.0" encoding="utf-8"?>
            <method>Trivial</method>
            <weightfile>fake.xml</weightfile>
            <treename>tree</treename>
            <target_variable>isSignal</target_variable>
            <weight_variable>__weight__</weight_variable>
            <signal_class>1</signal_class>
            <max_events>0</max_events>
            <number_feature_variables>1</number_feature_variables>
            <variable0>M</variable0>
            <number_spectator_variables>0</number_spectator_variables>
            <number_data_files>1</number_data_files>
            <datafile0>train.root</datafile0>
            <Trivial_version>1</Trivial_version>
            <Trivial_output>0</Trivial_output>
            <signal_fraction>0.066082567</signal_fraction>
            """
        with open("fake.xml", "w") as f:
            f.write(content)

        b2.process(path)
