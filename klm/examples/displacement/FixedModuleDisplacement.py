#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Generates EKLM displacement data with fixed module displacement.

import basf2
from ROOT import Belle2  # noqa: make the Belle2 namespace available
from ROOT.Belle2 import KLMDatabaseImporter, BKLMAlignment, EKLMAlignment, \
                        EKLMSegmentAlignment, KLMAlignmentData, \
                        KLMChannelIndex, KLMDisplacementGenerator

# Create main path
main = basf2.create_path()
basf2.set_log_level(basf2.LogLevel.INFO)

# EventInfoSetter
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Process the main path
basf2.process(main)

dbImporter = KLMDatabaseImporter()

bklmAlignment = BKLMAlignment()
eklmAlignment = EKLMAlignment()
eklmSegmentAlignment = EKLMSegmentAlignment()

# Fixed displacement for EKLM modules, zero displacement for EKLM segments.
displacementGenerator = KLMDisplacementGenerator()
displacementGenerator.fillZeroDisplacements(
    eklmAlignment, eklmSegmentAlignment)
displacementGenerator.generateFixedModuleDisplacement(1, 0, 0)
displacementGenerator.saveDisplacement(
    eklmAlignment, eklmSegmentAlignment, 'EKLMDisplacement.root')

# Zero displacement for BKLM.
alignmentData = KLMAlignmentData(0, 0, 0, 0, 0, 0)
index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
index = index2.beginBKLM()

while (index != index2.endBKLM()):
    module = index.getKLMModuleNumber()
    bklmAlignment.setModuleAlignment(module, alignmentData)
    index.increment()

# Import the payloads.
dbImporter.setIOV(0, 0, -1, -1)
dbImporter.importAlignment(
    bklmAlignment, eklmAlignment, eklmSegmentAlignment, True)
