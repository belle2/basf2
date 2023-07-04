#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import modularAnalysis as ma
import tempfile
import b2test_utils

from ROOT import Belle2

"""
Check the MVAExpertModule and MVAMultipleExpertsModule modules properly sets the extraInfo fields they should
 and that the overwrite settings are respected.
"""


class MVAExtraInfoChecker(basf2.Module):
    """Check if the extra Info values are correctly overwritten"""

    def initialize(self):
        """Create particle list object"""
        #: Particle list object
        self.plist = Belle2.PyStoreObj("pi+:test")
        #: Composite particle list object
        self.comp_plist = Belle2.PyStoreObj("Lambda0:test")

        #: event extra info object
        self.eventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

    def event(self):
        """check the extra info names are what we expect!"""
        for multiclass in [False, True]:
            for multiexpert_prefix in ['multi_', '']:
                for name, value in [('low_never', 0.5),
                                    ('low_always', 0.0),
                                    ('low_higher', 0.5),
                                    ('low_lower', 0.0),

                                    ('high_never', 0.5),
                                    ('high_always', 1.0),
                                    ('high_higher', 1.0),
                                    ('high_lower', 0.5)]:
                    name = multiexpert_prefix+name
                    if multiclass:
                        for index in range(3):
                            compare_val = index + value
                            extra_info_name = f'multiclass_{name}_{index}'
                            # FSP use case
                            for p in self.plist:
                                ei_value = p.getExtraInfo(extra_info_name)
                                assert ei_value == compare_val,\
                                    f'ExtraInfo "{extra_info_name}" value "{ei_value}" not what was expected {compare_val}'
                            # Decay string use case
                            for p in self.comp_plist:
                                if 'multi_' in extra_info_name:
                                    continue
                                ei_value = p.getDaughter(0).getExtraInfo(extra_info_name)
                                assert ei_value == compare_val,\
                                    f'ExtraInfo "{extra_info_name}" value "{ei_value}" not what was expected {compare_val}'

                            ei_value = self.eventExtraInfo.getExtraInfo(extra_info_name)
                            assert ei_value == compare_val,\
                                f'eventExtraInfo "{extra_info_name}" value "{ei_value}" not what was expected {compare_val}'
                    else:
                        extra_info_name = name
                        # FSP use case
                        for p in self.plist:
                            ei_value = p.getExtraInfo(name)
                            assert ei_value == value,\
                                f'ExtraInfo "{name}" value "{ei_value}" not what was expected "{value}"'
                        # Decay string use case
                        for p in self.comp_plist:
                            if 'multi_' in extra_info_name:
                                continue
                            ei_value = p.getDaughter(0).getExtraInfo(name)
                            assert ei_value == value,\
                                f'ExtraInfo "{name}" value "{ei_value}" not what was expected "{value}"'
                        ei_value = self.eventExtraInfo.getExtraInfo(name)
                        assert ei_value == value,\
                            f'eventExtraInfo "{name}" value "{ei_value}" not what was expected "{value}"'


# prepare the weightfiles
def create_weightfile(name, val):
    return f"""
        <?xml version="1.0" encoding="utf-8"?>
        <method>Trivial</method>
        <weightfile>{name}</weightfile>
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
        <Trivial_output>{val}</Trivial_output>
        <signal_fraction>0.5</signal_fraction>
        """


def create_multiclass_weightfile(name, offset_val):
    return f"""
        <?xml version="1.0" encoding="utf-8"?>
        <method>Trivial</method>
        <weightfile>{name}</weightfile>
        <treename>tree</treename>
        <target_variable>isSignal</target_variable>
        <weight_variable>__weight__</weight_variable>
        <signal_class>1</signal_class>
        <max_events>0</max_events>
        <nClasses>3</nClasses>
        <number_feature_variables>1</number_feature_variables>
        <variable0>beamE</variable0>
        <number_spectator_variables>0</number_spectator_variables>
        <number_data_files>1</number_data_files>
        <datafile0>train.root</datafile0>
        <Trivial_version>1</Trivial_version>
        <Trivial_output>0</Trivial_output>
        <Trivial_number_of_multiple_outputs>3</Trivial_number_of_multiple_outputs>
        <Trivial_multiple_output0>{offset_val + 0}</Trivial_multiple_output0>
        <Trivial_multiple_output1>{offset_val + 1}</Trivial_multiple_output1>
        <Trivial_multiple_output2>{offset_val + 2}</Trivial_multiple_output2>
        <signal_fraction>0.5</signal_fraction>
        """


if __name__ == "__main__":

    path = basf2.create_path()
    ma.inputMdst(b2test_utils.require_file('analysis/tests/mdst.root'), path=path)

    ma.fillParticleList('pi+:test', '', path=path)
    ma.fillParticleList('Lambda0:test -> p+ pi-', '', path=path)

    # test all combinations of [single expert, multiexpert] x [binary, multiclass]
    for prefix in ['', 'multiclass_']:
        for identifier, extra_info_name, overwrite_option in [
                # set the initial values
                ('weightfile_mid.xml', 'low_never', 0),
                ('weightfile_mid.xml', 'low_always', 2),
                ('weightfile_mid.xml', 'low_lower', -1),
                ('weightfile_mid.xml', 'low_higher', 1),
                ('weightfile_mid.xml', 'high_never', 0),
                ('weightfile_mid.xml', 'high_always', 2),
                ('weightfile_mid.xml', 'high_lower', -1),
                ('weightfile_mid.xml', 'high_higher', 1),
                # try to overwrite them
                ('weightfile_low.xml', 'low_never', 0),
                ('weightfile_low.xml', 'low_always', 2),
                ('weightfile_low.xml', 'low_lower', -1),
                ('weightfile_low.xml', 'low_higher', 1),
                ('weightfile_high.xml', 'high_never', 0),
                ('weightfile_high.xml', 'high_always', 2),
                ('weightfile_high.xml', 'high_lower', -1),
                ('weightfile_high.xml', 'high_higher', 1),
                ]:
            extra_info_name = prefix+extra_info_name
            identifier = prefix+identifier
            path.add_module(
                'MVAExpert',
                listNames=['pi+:test'],
                extraInfoName=extra_info_name,
                identifier=identifier,
                overwriteExistingExtraInfo=overwrite_option)
            path.add_module(
                'MVAExpert',
                listNames=['Lambda0:test -> ^p+ pi-'],
                extraInfoName=extra_info_name,
                identifier=identifier,
                overwriteExistingExtraInfo=overwrite_option)
            path.add_module(
                'MVAExpert',
                listNames=[],
                extraInfoName=extra_info_name,
                identifier=identifier,
                overwriteExistingExtraInfo=overwrite_option)

        for identifiers, extra_info_names, overwrite_options in [
                (['weightfile_mid.xml', 'weightfile_mid.xml'], ['multi_low_never', 'multi_high_never'], [0, 0]),
                (['weightfile_mid.xml', 'weightfile_mid.xml'], ['multi_low_always', 'multi_high_always'], [2, 2]),
                (['weightfile_mid.xml', 'weightfile_mid.xml'], ['multi_low_lower', 'multi_high_lower'], [-1, -1]),
                (['weightfile_mid.xml', 'weightfile_mid.xml'], ['multi_low_higher', 'multi_high_higher'], [1, 1]),
                (['weightfile_low.xml', 'weightfile_high.xml'], ['multi_low_never', 'multi_high_never'], [0, 0]),
                (['weightfile_low.xml', 'weightfile_high.xml'], ['multi_low_always', 'multi_high_always'], [2, 2]),
                (['weightfile_low.xml', 'weightfile_high.xml'], ['multi_low_higher', 'multi_high_lower'], [1, -1]),
                (['weightfile_low.xml', 'weightfile_high.xml'], ['multi_low_lower', 'multi_high_higher'], [-1, 1]),
                ]:
            extra_info_names = [prefix+x for x in extra_info_names]
            identifiers = [prefix+x for x in identifiers]
            path.add_module(
                'MVAMultipleExperts',
                listNames=['pi+:test'],
                extraInfoNames=extra_info_names,
                identifiers=identifiers,
                overwriteExistingExtraInfo=overwrite_options)
            path.add_module('MVAMultipleExperts', listNames=[], extraInfoNames=extra_info_names,
                            identifiers=identifiers, overwriteExistingExtraInfo=overwrite_options)

    path.add_module(MVAExtraInfoChecker())

    with tempfile.TemporaryDirectory() as tempdir:
        for name, val in [('weightfile_low.xml', 0.0),
                          ('weightfile_mid.xml', 0.5),
                          ('weightfile_high.xml', 1.0)]:
            with open(name, "w") as f:
                f.write(create_weightfile(name, val))

        for name, val in [('multiclass_weightfile_low.xml', 0.0),
                          ('multiclass_weightfile_mid.xml', 0.5),
                          ('multiclass_weightfile_high.xml', 1.0)]:
            with open(name, "w") as f:
                f.write(create_multiclass_weightfile(name, val))

        basf2.process(path, 10)
