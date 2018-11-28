#####################################################################################
#
#
#
#
#
#  This module prints out the AVERAGE preformance PER MC samplesof the different skimming scripts
#
#        By Racha Cheaib Dec 1st, 2018
################################################################################

# !/usr/bin/env python3
from basf2 import *
import os
import itertools
import sys
import collections
from skimExpertFunctions import *
import subprocess
import json


skims = 'TauLFV ALP3Gamma BottomoniumEtabExclusive'
#  skimNames1 = ' BtoDh_Kspi0 BtoDh_Kspipipi0 BtoDh_Kshh BtoDh_hh BtoPi0Pi0
# BottomoniumEtabExclusive BottomoniumUpsilon  SLUntagged LeptonicUntagged
# skimName2 = ' Charm2BodyHadronic Charm3BodyHadronic Charm3BodyHadronic2
# Charm2BodyHadronicD0 CharmSemileptonic  Charm2BodyNeutrals
# Charm2BodyNeutralsD0 CharmRare CharmlessHad SystematicsJpsiee'
# skiName3 = ' SystematicsJpsimumu SystematicsDstar Tau TCPV
#  PRsemileptonicUntagged DoubleCharm feiHadronicB0 feiHadronicBplus
# feiSLB0WithOneLep feiSLBplusWithOneLep '

bkgs = 'MC9_mixedBGx1  MC9_chargedBGx1 MC9_ccbarBGx1 MC9_ssbarBGx1 MC9_uubarBGx0  MC9_ddbarBGx1  MC9_taupairBGx1'
# ' MC9_ mixedBGx0 MC9_chargedBGx0 MC9_ccbarBGx0 MC9_ssbarBGx0 MC9_uubarBGx0 MC9_ddbarBGx0 MC9_taupairBGx0'


nFullFiles = 10000
nFullEvents = 200000
nSkims = 0


totalUdstSizePerEventPerSample = 0
totalRetentionPerSample = 0
totalProcessingTimePerEventPerSample = 0

avgUdstSizePerEventPerSample = 0
avgRetentionPerSample = 0
avgProcessingTimePerEventPerSample = 0

for bkg in bkgs.split():
    print('|Bkg:' + bkg)
    title = '|Skim       |  Total   Retention   | Total Time/Evt(HEPSEC)| Total uDSTSize/Evt(KB)|'

    title += '  Average   Retention   | Average Time/Evt(HEPSEC)| Average uDSTSize/Evt(KB)|'
    print(title)
    for skim in skims.split():
        inputFileName = skim + '_' + bkg + '.out'
        outputFileName = skim + '_' + bkg
        outputUdstName = skim + '_' + bkg
        nSkims += 1
        pos = bkg.find('_')
        skimCampaign = bkg[0:pos]
        sampleType = bkg[pos + 1:]
        fileList = getTestFile(sampleType, skimCampaign)
        nFullEvents = getNEvents(fileList)
        nSkimmedEvents = getNEvents(outputUdstName + '.udst.root')
        nFullFiles = getNumberOfInputMdstFilesPerSample(sampleType, skimCampaign)
        # These counters are included to determine the number  of lines with retention and candidate multiplicity information.
        lineCounter = 0
        l = 0
        events = 10000
        timePerEvent = 0
        retention = 0
        udstSizeByte = 0
        udstSizePerEvent = 0
        udstSizePerEvent = 0
        totalTime = 0
        with open(inputFileName, 'r') as inF:
            content = []
            for line in inF:
                lineCounter += 1
                content.append(line)
                if 'Total                ' in line:  # find line with total time
                    tline = line.split()
                    time = float(tline[6]) * 23.57  # In hepsec: 23.57 hepsec/core
                    events = float(tline[2])
                    timePerEvent = time / events
                if 'Max Memory ' in line:
                    maxMemline = line.split()
                    maxMemory = float(maxMemline[3]) * 1000
                    maxMemoryPerEvent = maxMemory / events

                if 'Average Memory' in line:
                    avgMemline = line.split()
                    avgMemory = float(avgMemline[3]) * 1000
                    avgMemoryPerEvent = avgMemory / events
                if 'Total Retention' in line:  # Find line with total retention
                    rline = line.split(' ')
                    retention = float(rline[2])  # print('Total retention is ' + str(retention))

        statinfo_udst = os.stat(outputUdstName + '.udst.root')
        udstSizeByte = str(statinfo_udst.st_size)
        udstSizeKiloByte = statinfo_udst.st_size / 1024
        if (retention != 0):
            udstSizePerEvent = udstSizeKiloByte / (events * retention)

        totalTime = timePerEvent * nFullEvents / 23.57
        totalRetentionPerSample += retention
        totalUdstSizePerEventPerSample += udstSizePerEvent
        totalProcessingTimePerEventPerSample += timePerEvent

    print('Average calculated over ' + str(nSkims) + ' skims')
    avgRetentionPerSample = totalRetentionPerSample / nSkims
    avgUdstSizePerEventPerSample = totalUdstSizePerEventPerSample / nSkims
    avgProcessingTimePerEventPerSample = totalProcessingTimePerEventPerSample / nSkims

    result = '|' + skim + '    |  ' + str(totalRetentionPerSample) + ' | ' + \
        str(totalProcessingTimePerEventPerSample) + '  |  ' + str(totalUdstSizePerEventPerSample) + '|'

    result += ' ' + str(avgRetentionPerSample) + ' | ' + str(avgProcessingTimePerEventPerSample) + \
        '  |  ' + str(avgUdstSizePerEventPerSample) + '|'
    print(result)
