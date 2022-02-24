#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
import rawdata as raw
import reconstruction as re
from daqdqm.commondqm import add_common_dqm
import basf2 as b2

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

files = ["/group/belle2/dataprod/Data/Raw/e0018/r01553/sub00/physics.0018.01553.HLT8.f0000*.root"]  # might require modifying

# Create main path
main = b2.create_path()
b2.conditions.globaltags = ['online', 'dp_recon_release6_patch',
                            'ecl_release06_forOnline']  # might require modifying for not release-06 tags

main.add_module("RootInput", inputFileNames=files)
main.add_module("HistoManager", histoFileName="KLMDQMHistograms.root")
main.add_module('Gearbox')
main.add_module('Geometry')
raw.add_unpackers(main)
re.add_reconstruction(main, add_muid_hits=True)

add_common_dqm(main, dqm_environment='HLT', dqm_mode='dont_care')
ma.fillParticleList('mu+:all', cut="", path=main)
ma.cutAndCopyList("mu+:cut", "mu+:all", 'SoftwareTriggerResult(software_trigger_cut&skim&accept_mumutight)>0', path=main)

collector = b2.register_module('KLMStripEfficiencyCollector')
collector.param('Debug', True)
collector.param('DebugFileName', 'KLMStripEffCollector_debugFile.root')
collector.param('MuonListName', 'mu+:cut')  # this cut is needed to be consistent with KLMDQM2
main.add_module(collector)

klm2 = b2.register_module('KLMDQM2')
klm2.param('Debug', True)
klm2.param("DebugFileName", "KLMDQM2_debugFile.root")
main.add_module(klm2)

main.add_module('Progress')
b2.print_path(main)


# Process all events
b2.process(main, max_event=1000)
print(b2.statistics)
