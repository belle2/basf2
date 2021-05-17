#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""This steering file creates ECL DQM histograms from
   input SeqRoot file

The user should provide input and output file names and
control 'ECLDQM' module parameters.

Usage:
    $ basf2 -i <path_to_input_files> -o <path_to_output_files>
            EclDQM.py
"""

import basf2 as b2

__author__ = 'Dmitry Matvienko'
__copyright__ = 'Copyright 2019 - Belle II Collaboration'
__maintainer__ = 'Dmitry Matvienko'
__email__ = 'd.v.matvienko@inp.nsk.su'

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

b2.set_log_level(b2.LogLevel.ERROR)

# Register and add 'SeqRootInput' module
seqRootInput = b2.register_module('SeqRootInput')
mainPath.add_module(seqRootInput)

# Register and add 'ECLUnpacker' module
eclUnpacker = b2.register_module('ECLUnpacker')
eclUnpacker.param('storeTrigTime', True)
mainPath.add_module(eclUnpacker)

# Register and add 'ECLDigitCalibrator' module
eclDigitCalibrator = b2.register_module('ECLDigitCalibrator')
mainPath.add_module(eclDigitCalibrator)

# Register and add 'HistoManager' module
histoManager = b2.register_module('HistoManager')
mainPath.add_module(histoManager)

# Register and add 'TriggerSkim' module and settings
triggerSkim = b2.register_module('TriggerSkim')
triggerSkim.param('triggerLines',
                  ['software_trigger_cut&all&total_result'])
triggerSkim.if_value('==0', b2.Path(), b2.AfterConditionPath.END)
mainPath.add_module(triggerSkim)

# Register and add 'ECLDQM' module and settings
eclDQM = b2.register_module('ECLDQM')
eclDQM.param('histogramDirectoryName', 'ECL')
eclDQM.param('EnergyUpperThr', 1.5)
eclDQM.param('PedestalMeanUpperThr', 7000)
eclDQM.param('PedestalMeanLowerThr', -1000)
eclDQM.param('PedestalRmsUpperThr', 100.)
mainPath.add_module(eclDQM)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
