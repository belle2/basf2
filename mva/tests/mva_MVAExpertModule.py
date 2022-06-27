#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check the MVAExpertModule and MVAMultipleExpertsModule modules properly sets the extraInfo fields they should.
"""
import basf2 as b2
import b2test_utils
import modularAnalysis as ma

import tempfile
import os

from ROOT import Belle2


class MVAExtraInfoChecker(b2.Module):
    """Check if the extra Info values are set correctly by the MVAExpertModules"""

    def initialize(self):
        """Create particle list object"""
        #: particle list object
        self.plist = Belle2.PyStoreObj("pi+:test")
        #: event extra info object
        self.eventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

    def event(self):
        """check the extra info names are what we expect!"""
        for p in self.plist:
            for name, value in [
                                ('singleClassSingleExpert', 42),
                                ('multiClassSingleExpert_0', 0),
                                ('multiClassSingleExpert_1', 1),
                                ('multiClassSingleExpert_2', 2),
                                ('multiClassSingleExpert_3', 3),

                                ('singleClassMultiExpert', 42),
                                ('multiClassMultiExpert_0', 0),
                                ('multiClassMultiExpert_1', 1),
                                ('multiClassMultiExpert_2', 2),
                                ('multiClassMultiExpert_3', 3),
                               ]:
                assert p.getExtraInfo(name) == value, 'ExtraInfo value not what was expected'

        for name, value in [
                            ('singleClassSingleExpert', 42),
                            ('multiClassSingleExpert_0', 0),
                            ('multiClassSingleExpert_1', 1),
                            ('multiClassSingleExpert_2', 2),
                            ('multiClassSingleExpert_3', 3),

                            ('singleClassMultiExpert', 42),
                            ('multiClassMultiExpert_0', 0),
                            ('multiClassMultiExpert_1', 1),
                            ('multiClassMultiExpert_2', 2),
                            ('multiClassMultiExpert_3', 3),
                           ]:
            assert self.eventExtraInfo.getExtraInfo(name) == value, 'pventExtraInfo value not what was expected'


if __name__ == "__main__":

    path = b2.create_path()

    ma.inputMdst(b2test_utils.require_file('analysis/tests/mdst.root'), path=path)
    ma.fillParticleList('pi+:test', '', path=path)

    # test writing to particle extraInfo
    path.add_module('MVAExpert', listNames=['pi+:test'],
                    extraInfoName='singleClassSingleExpert',
                    identifier='singleClass.xml')
    path.add_module('MVAExpert', listNames=['pi+:test'],
                    extraInfoName='multiClassSingleExpert',
                    identifier='multiClass.xml')
    path.add_module('MVAMultipleExperts', listNames=['pi+:test'],
                    extraInfoNames=['singleClassMultiExpert', 'multiClassMultiExpert'],
                    identifiers=['singleClass.xml', 'multiClass.xml'])

    # test writing to eventExtraInfo
    path.add_module('MVAExpert', listNames=[],
                    extraInfoName='singleClassSingleExpert',
                    identifier='singleClass.xml')
    path.add_module('MVAExpert', listNames=[],
                    extraInfoName='multiClassSingleExpert',
                    identifier='multiClass.xml')
    path.add_module('MVAMultipleExperts', listNames=[],
                    extraInfoNames=['singleClassMultiExpert', 'multiClassMultiExpert'],
                    identifiers=['singleClass.xml', 'multiClass.xml'])

    path.add_module(MVAExtraInfoChecker())

    with tempfile.TemporaryDirectory() as tempdir:
        os.chdir(tempdir)
        # Create a fake weightfile
        content_single = """
            <?xml version="1.0" encoding="utf-8"?>
            <method>Trivial</method>
            <weightfile>singleClass.xml</weightfile>
            <treename>tree</treename>
            <target_variable>isSignal</target_variable>
            <weight_variable>__weight__</weight_variable>
            <signal_class>1</signal_class>
            <max_events>0</max_events>
            <number_feature_variables>1</number_feature_variables>
            <variable0>beamE</variable0>
            <number_spectator_variables>0</number_spectator_variables>
            <number_data_files>1</number_data_files>
            <datafile0>train.root</datafile0>
            <Trivial_version>1</Trivial_version>
            <Trivial_output>42</Trivial_output>
            <signal_fraction>0.5</signal_fraction>
            """
        with open("singleClass.xml", "w") as f:
            f.write(content_single)

        # Create a fake weightfile
        content_multi = """
            <?xml version="1.0" encoding="utf-8"?>
            <method>Trivial</method>
            <weightfile>multiClass.xml</weightfile>
            <treename>tree</treename>
            <target_variable>isSignal</target_variable>
            <weight_variable>__weight__</weight_variable>
            <signal_class>1</signal_class>
            <max_events>0</max_events>
            <nClasses>4</nClasses>
            <number_feature_variables>1</number_feature_variables>
            <variable0>beamE</variable0>
            <number_spectator_variables>0</number_spectator_variables>
            <number_data_files>1</number_data_files>
            <datafile0>train.root</datafile0>
            <Trivial_version>1</Trivial_version>
            <Trivial_output>0</Trivial_output>
            <Trivial_number_of_multiple_outputs>4</Trivial_number_of_multiple_outputs>
            <Trivial_multiple_output0>0</Trivial_multiple_output0>
            <Trivial_multiple_output1>1</Trivial_multiple_output1>
            <Trivial_multiple_output2>2</Trivial_multiple_output2>
            <Trivial_multiple_output3>3</Trivial_multiple_output3>
            <signal_fraction>0.5</signal_fraction>
            """
        with open("multiClass.xml", "w") as f:
            f.write(content_multi)

        b2.process(path, 10)
