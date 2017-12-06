#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Thomas Keck 2016

from basf2 import *

from modularAnalysis import *

import tempfile
import os

from ROOT import Belle2

if __name__ == "__main__":

    path = create_path()
    generateY4S(100, Belle2.FileSystem.findFile('analysis/examples/tutorials/B2A101-Y4SEventGeneration.dec'), path=path)
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

        process(path)
