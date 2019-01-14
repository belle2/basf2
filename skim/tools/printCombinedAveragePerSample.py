
# !/usr/bin/env python3


"""
This is a tool that prints out the follwing information about combined skims per MC sample type:
   1) Total Retention Rate
   2) Total Time/Event  (HEPSEC)
   3) Total uDST size /Event (kB)

   1) Average Retention Rate
   2) Average Time/Event  (HEPSEC)
   3) Average uDST size /Event (kB)



To run printCombinedAvgPerSample.py, you need to have run your skim on a set of input mDST files
(you can use runCombinedSkims.py), and produce a set of uDST and log files with the following
name scheme:
  SkimName_SampleName.udst.root
  SkimName_SampleName.out

"""

__author__ = " R. Cheaib"


from basf2 import *
import os
import itertools
import sys
import collections
from skimExpertFunctions import get_test_file, get_eventN

skims = 'Dark Semileptonic feiHadronic BtoCharm BtoCharmless Quarkonium  CombinedSystematics MiscCombined'
bkgs = 'mixedBGx1  chargedBGx1 ccbarBGx1 ssbarBGx1 uubarBGx0  ddbarBGx1  taupairBGx1'
bkgs += ' mixedBGx0 chargedBGx0 ccbarBGx0 ssbarBGx0 uubarBGx0 ddbarBGx0 taupairBGx0'

nFullEvents = 200000
nSkims = 0


for bkg in bkgs.split():
    print('|Bkg:' + bkg)
    title = '|Sample       |  Total   Retention   | Total Time/Evt(HEPSEC)| Total uDSTSize/Evt(KB)|'

    title += '  Average   Retention   | Average Time/Evt(HEPSEC)| Average uDSTSize/Evt(KB)|'
    print(title)

    totalUdstSizePerEventPerSample = 0
    totalRetentionPerSample = 0
    totalProcessingTimePerEventPerSample = 0
    totalUdstSizePerEvent = 0
    avgUdstSizePerEventPerSample = 0
    avgRetentionPerSample = 0
    avgProcessingTimePerEventPerSample = 0
    nSkims = 0
    totalRetention = 0
    for skim in skims.split():

        if (skim == 'CombinedSystematics'):
            nSkims = 6
            partSkim = 'Systematics SystematicsLambda SystematicsTracking Resonance SystematicsRadMuMu SystematicsRadEE'

        if (skim == 'Semileptonic'):
            nSkims = 3
            partSkim = 'PRsemileptonicUntagged LeptonicUntagged SLUntagged'

        if (skim == 'MiscCombined'):
            nSkims = 3
            partSkim = 'BtoPi0Pi0 TauGeneric TauLFV'

        if (skim == 'Dark'):
            nSkims = 4
            partSkim = 'ALP3Gamma SinglePhotonDark LFVZpVisible LFVZpInvisible'

        if (skim == 'feiHadronic'):
            nSkims = 2
            partSkim = 'feiHadronicB0 feiHadronicBplus'

        if (skim == 'Quarkonium'):
            nSkims = 3
            partSkim = 'ISRpipicc BottomoniumEtabExclusive BottomoniumUpsilon'

        if (skim == 'BtoCharmless'):
            nSkims = 2
            partSkim = 'CharmlessHad2Body CharmlessHad3Body'

        if (skim == 'BtoCharm'):
            nSkims = 4
            partSkim = 'BtoDh_Kshh BtoDh_hh BtoDh_Kspi0 BtoDh_Kspipipi0'

        inputFileName = skim + '_' + bkg + '.out'
        outputFileName = skim + '_' + bkg
        outputUdstName = '../standalone/outputFiles/' + partSkim + '_' + bkg
        nSkims += 1
        pos = bkg.find('_')
        skimCampaign = bkg[0:pos]
        sampleType = bkg[pos + 1:]
        skimCampaign = 'MC9'
        fileList = get_test_file(sampleType, skimCampaign)
        nFullEvents = get_eventN(fileList)
        nSkimmedEvents = 0  # get_eventN(outputFileName + '.udst.root')
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
                if l == 0 and 'Total Retention' in line:  # Find line with total retention
                    l += 1
                    rline = line.split(' ')
                    retention = float(rline[2])
                    totalRetention += retention  # print('Total retention is ' + str(retention))

                if l == 1 and 'Total Retention' in line:  # Find line with total retention
                    rline = line.split(' ')
                    tempr = float(rline[2])
                    if (tempr != retention):
                        totalRetention += float(rline[2])
            for part in partSkim.split():
                outputUdstName = '../standalone/outputFiles/' + part + '_' + bkg
                nPartSkimmedEvents = get_eventN(outputUdstName + '.udst.root')
                statinfo_udst = os.stat(outputUdstName + '.udst.root')
                udstSizeByte = str(statinfo_udst.st_size)
                udstSizeKiloByte = statinfo_udst.st_size / 1024
                if (retention != 0):
                    udstSizePerEvent = udstSizeKiloByte / (events * retention)
                totalUdstSizePerEvent += udstSizePerEvent
        totalTime = timePerEvent * nFullEvents / 23.57
        totalRetentionPerSample += totalRetention
        totalUdstSizePerEventPerSample += totalUdstSizePerEvent
        totalProcessingTimePerEventPerSample += timePerEvent
    avgRetentionPerSample = totalRetentionPerSample / nSkims
    avgUdstSizePerEventPerSample = totalUdstSizePerEventPerSample / nSkims
    avgProcessingTimePerEventPerSample = totalProcessingTimePerEventPerSample / nSkims

    result = '|' + bkg + '    |  ' + str(totalRetentionPerSample) + ' | ' + \
        str(totalProcessingTimePerEventPerSample) + '  |  ' + str(totalUdstSizePerEventPerSample) + '|'

    result += ' ' + str(avgRetentionPerSample) + ' | ' + str(avgProcessingTimePerEventPerSample) + \
        '  |  ' + str(avgUdstSizePerEventPerSample) + '|'
    print(result)
