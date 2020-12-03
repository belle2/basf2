#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM alignment and displacement payloads.

import sys
import basf2
from ROOT.Belle2 import KLMDatabaseImporter, BKLMAlignment, EKLMAlignment, \
                        EKLMSegmentAlignment, KLMAlignmentData, \
                        KLMElementNumbers, KLMChannelIndex

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = KLMDatabaseImporter()

bklmAlignment = BKLMAlignment()
eklmAlignment = EKLMAlignment()
eklmSegmentAlignment = EKLMSegmentAlignment()

alignmentData = KLMAlignmentData(0, 0, 0, 0, 0, 0)

index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)

while (index != index2.end()):
    module = index.getKLMModuleNumber()
    if (index.getSubdetector() == KLMElementNumbers.c_BKLM):
        bklmAlignment.setModuleAlignment(module, alignmentData)
    else:
        eklmAlignment.setModuleAlignment(module, alignmentData)
    index.increment()

index.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
index2.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
index = index2.beginEKLM()
index.useEKLMSegments()
while (index != index2.endEKLM()):
    segment = index.getEKLMSegmentNumber()
    eklmSegmentAlignment.setSegmentAlignment(segment, alignmentData)
    index.increment()

if (mc):
    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment, True)

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment, True)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment, True)

else:
    dbImporter.setIOV(0, 0, -1, -1)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment)
    dbImporter.importAlignment(bklmAlignment, eklmAlignment,
                               eklmSegmentAlignment, True)
