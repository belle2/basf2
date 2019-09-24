# !/usr/bin/env python3


"""
 This is a tool that prints out the follwing information about a combined skim:

   1) Retention Rate
   2) Number of Input Events
   3) Number of Skimmed Events
   4) Time/Event  (HEPSEC)
   5) Estimated total Time (s)
   6) uDST size /event (kB)
   7) Estimated total udst size (kB)
   8) Log file size /event (kB)
   9) Estimated total log file size (MB)
   10) Average Memory usage (GB)
   11) Max Memory Usage (GB)
   12) Estimated total uDST filesize of a skim and sample (GB).
      (taking into account the total number of input mdst files available per sample.)
  13) Estimated total log file size  of a skim and a sample (GB).
      (taking into account the total number of input mdst files available per sample.)

 To run printCombinedStats.py, you need to have run your skim on a set of input mDST files
 (you can use runCombinedStats.py), and produce a set of uDST and log files with the following
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
from skimExpertFunctions import get_eventN, get_total_infiles, get_test_file, encodeSkimName
import subprocess
import json


skims = 'Dark Semileptonic BtoCharm BtoCharmless Quarkonium EWP CombinedSystematics MiscCombined feiHadronicCombined feiSLCombined'

bkgs = 'MC12_chargedBGx1  MC12_chargedBGx1 MC12_ccbarBGx1 MC12_ssbarBGx1 MC12_uubarBGx0  MC12_ddbarBGx1  MC12_taupairBGx1'
bkgs += ' MC12_mixedBGx0 MC12_chargedBGx0 MC12_ccbarBGx0 MC12_ssbarBGx0 MC12_uubarBGx0 MC12_ddbarBGx0 MC12_taupairBGx0'
jsonMergeFactorInput = open('JsonMergeFactorInput.txt', 'w')
jsonEvtSizeInput = open('JsonEvtSizeInput.txt', 'w')
jsonTimeInput = open('JsonTimeInput.txt', 'w')

nFullFiles = 10000
nFullEvents = 200000
avgUdstSizePerEventPerSample = 0
avgRetentionPerSample = 0
avgProcessingTimePerEventPerSample = 0
for skim in skims.split():
    jsonTimeInput.write('t_' + skim + '=[')
    jsonEvtSizeInput.write('s_' + skim + '=[')
    jsonMergeFactorInput.write('m_' + skim + '=[')

    if (skim == 'CombinedSystematics'):
        nSkims = 6
        partSkim = 'Systematics SystematicsLambda SystematicsTracking Resonance SystematicsRadMuMu SystematicsRadEE'

    if (skim == 'Semileptonic'):
        nSkims = 3
        partSkim = 'PRsemileptonicUntagged LeptonicUntagged SLUntagged'

    if (skim == 'MiscCombined'):
        nSkims = 3
        partSkim = 'TauThrust TauGeneric TauLFV'

    if (skim == 'Dark'):
        nSkims = 6
        partSkim = 'ALP3Gamma SinglePhotonDark LFVZpVisible DimuonPlusMissingEnergy'
        partSkim += ' ElectronMuonPlusMissingEnergy DielectronPlusMissingEnergy'

    if (skim == 'feiHadronicCombined'):
        nSkims = 2
        partSkim = 'feiHadronicB0 feiHadronicBplus'

    if (skim == 'feiSLCombined'):
        nSkims = 2
        partSkim = 'feiSLB0 feiSLBplus'
    if (skim == 'Quarkonium'):
        nSkims = 3
        partSkim = 'ISRpipicc BottomoniumEtabExclusive BottomoniumUpsilon'

    if (skim == 'BtoCharmless'):
        nSkims = 2
        partSkim = 'CharmlessHad2Body CharmlessHad3Body'

    if (skim == 'BtoCharm'):
        nSkims = 4
        partSkim = 'BtoDh_Kshh BtoDh_hh BtoDh_Kspi0 BtoDh_Kspipipi0'
    print('|Skim:' + skim + '_Skim_Standalone Statistics|')
    title = '|Bkg        |  InputEvents  |  Skimmed Events  |   Retention   | Time/Evt(HEPSEC)| Total Time (s) |uDSTSize/Evt(KB)|'
    title += ' uDSTSize(MB)|  ACMPE   |Log Size/evt(KB)|Log Size(MB)|'
    title += ' MaxMemory (GB)  | AvgMemory (GB) | FullSkimSize(GB)|'
    title += ' FullSkimLogSize(GB)|'
    print(title)
    for bkg in bkgs.split():
        inputFileName = skim + '_' + bkg + '.out'
        nSkimmedEvents = 0
        nPartSkimmedEvents = 0
        pos = bkg.find('_')
        skimCampaign = bkg[0:pos]
        sampleType = bkg[pos + 1:]
        fileList = get_test_file(sampleType, skimCampaign)
        nFullEvents = get_eventN(fileList)
        nFullFiles = get_total_infiles(sampleType, skimCampaign)
        # These counters are included to determine the number  of lines with retention and candidate multiplicity information.
        lineCounter = 0
        l = 0
        a = 0  # Line number of first occurrence of retention value
        z = 0  # Line number of second occurrence retention value
        s = 0  # Line number of  the start of the mode list  candidate multiplicity determination
        f = 0  # Line number of the end of the mode list for candidate multiplcity determination
        acmCounter = 0
        acmN = 0
        acmD = 0
        time = 0.0
        events = 10000
        nModes = 0
        skipped = False
        timePerEvent = 0
        retention = 0
        totalRetention = 0
        mdstSizeByte = 0
        mdstSizeKiloByte = 0
        mdstSizePerEvent = 0
        udstSizeByte = 0
        udstSizePerEvent = 0
        totalUdstSizePerEvent = 0
        totalTime = 0
        maxMemory = 0
        diff = 1
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
                    maxMemory = float(maxMemline[3]) / 1000
                    maxMemoryPerEvent = maxMemory / events

                if 'Average Memory' in line:
                    avgMemline = line.split()
                    avgMemory = float(avgMemline[3]) / 1000
                    avgMemoryPerEvent = avgMemory / events
                if l == 0 and 'Total Retention' in line:  # Find line with total retention
                    l += 1
                    rline = line.split(' ')
                    retention = float(rline[2])
                    totalRetention += retention
                if l == 1 and 'Total Retention' in line:  # Find line with total retention
                    rline = line.split(' ')
                    tempr = float(rline[2])
                    if (tempr != retention):
                        totalRetention += float(rline[2])
                        retention = tempr

                if z == 0 and line.find('Candidate Multiplicity') >= 0:  # Find line with Candidate Multiplicity
                    z += 1
                    s = int(lineCounter) + 2
                if a == 0 and line.find('Total Retention') >= 0:
                    a += 1
                    f = int(lineCounter) - 3

# Candidate multiplicity is printed out per decay mode, We need the average so
# we count how many modes (i.e. lines) and then we use that as a
# denominator to determine the average candidate multiplicity
        if (s != 0 and f != 0):
            diff = f - s - 1  # Here diff should give you number of modes that you need to average over
        for i in range(0, diff):
            sline = content[s + i].split()
            if (i < 10):
                acmTemp = sline[3]
                if acmTemp[:1].isdigit():
                    acmN = acmN + float(acmTemp)
                else:
                    acmN = acmN + 0
                    nModes += 1
                    skipped = True
            elif (i > 9):
                acmTemp = sline[2]
                if acmTemp[:1].isdigit():
                    acmN = acmN + float(sline[2])
                else:
                    acmN = acmN + 0
                    nModes = +1
        acm = acmN / diff  # Determine the average candidate multiplicity
        # GET AND PRINT UDST EVENT SIZE:
        if retention == 0:
            udstSizePerEvent = 0
        if retention == 0:
            mdstSizePerEvent = 0
        for part in partSkim.split():
            outputUdstName = part + '_' + bkg

            nPartSkimmedEvents = get_eventN(outputUdstName + '.udst.root')
            statinfo_udst = os.stat(outputUdstName + '.udst.root')
            udstSizeByte = str(statinfo_udst.st_size)
            udstSizeKiloByte = statinfo_udst.st_size / 1024
            if (retention != 0):
                udstSizePerEvent = udstSizeKiloByte / (events * retention)
            totalUdstSizePerEvent += udstSizePerEvent

        statinfo_log = os.stat(inputFileName)
        logFileSizeByte = str(statinfo_log.st_size)
        logFileSizeKiloByte = statinfo_log.st_size / 1024
        logFileSizePerEvent = logFileSizeKiloByte / events

        fullFileSizeKB = totalUdstSizePerEvent * retention * nFullEvents
        fullFileSizeMB = fullFileSizeKB / 1000

        fullSkimSizeMB = fullFileSizeMB * nFullFiles
        fullSkimSizeGB = fullSkimSizeMB / 1000
        fullLogFileMB = logFileSizeKiloByte / 1000
        fullLogSkimSizeGB = fullLogFileMB * nFullFiles / 1000

        totalTime = timePerEvent * nFullEvents / 23.57
        print('|' +
              bkg +
              '     |     ' +
              str(nFullEvents) +
              '     |     ' +
              str(nSkimmedEvents) +
              '     |     ' +
              str(totalRetention) +
              '     |     ' +
              str(timePerEvent)[:5] +
              '      |     ' +

              str(totalTime)[:5] +
              '      |     ' +
              str(totalUdstSizePerEvent)[:5] +
              '     |     ' +
              str(fullFileSizeMB)[:5] +
              '    |   ' +
              str(acm)[:5] +
              '     |     ' +
              str(logFileSizePerEvent)[:5] +
              '     |     ' +
              str(fullLogFileMB)[:5] +
              '     |    ' +
              str(maxMemory)[:5] +
              '     |      ' +
              str(avgMemory)[:5] +
              '      |      ' +
              str(fullSkimSizeGB)[:5] +
              '   |    ' +
              str(fullLogSkimSizeGB)[:5] +
              "\n")
        outTimePerEvent = timePerEvent
        if (timePerEvent >= 0.5):
            outTimePerEvent = timePerEvent + 1
        elif (timePerEvent >= 0.1 and timePerEvent < 0.5):
            outTimePerEvent = timePerEvent + 0.5
        elif (timePerEvent >= 0.01 and timePerEvent < 0.1):
            outTimePerEvent = timePerEvent + 0.1
        elif (timePerEvent <= 0.01):
            outTimePerEvent = timePerEvent + 0.1

        outUdstSizePerEvent = udstSizePerEvent
        if (udstSizePerEvent >= 100):
            outUdstSizePerEvent = udstSizePerEvent + 10
        elif (udstSizePerEvent >= 10 and udstSizePerEvent < 100):
            outUdstSizePerEvent = udstSizePerEvent + 5
        elif (udstSizePerEvent < 10):
            outUdstSizePerEvent = udstSizePerEvent + 1

        outFullFileSize = fullFileSizeMB + 0.01
        if (fullFileSizeMB >= 100):
            outFullFileSize = fullFileSizeMB + 5
        elif (fullFileSizeMB >= 10 and fullFileSizeMB < 100):
            outFullFileSize = fullFileSizeMB + 2
        elif (fullFileSizeMB >= 1 and fullFileSizeMB < 10):
            outFullFileSize = fullFileSizeMB + 1
        elif (fullFileSizeMB >= 0.1 and fullFileSizeMB < 1):
            outFullFileSize = fullFileSizeMB + 0.05
        elif (fullFileSizeMB >= 0.01 and fullFileSizeMB < 0.1):
            outFullFileSize = fullFileSizeMB + 0.01
        elif (fullFileSizeMB < 0.01):
            outFullFileSize = fullFileSizeMB + 0.01

        jsonTimeInput.write(str(outTimePerEvent)[:3])

        if (outUdstSizePerEvent >= 100):
            jsonEvtSizeInput.write(str(outUdstSizePerEvent)[:3])
        elif (outUdstSizePerEvent >= 10 and outUdstSizePerEvent < 100):
            jsonEvtSizeInput.write(str(outUdstSizePerEvent)[:2])
        elif (outUdstSizePerEvent < 10):
            jsonEvtSizeInput.write(str(outUdstSizePerEvent)[:3])

        # print("FULLFILESIZE "+str(outFullFileSize))
        if (outFullFileSize >= 100):
            jsonMergeFactorInput.write(str(outFullFileSize)[:3])
        elif (outFullFileSize >= 10 and outFullFileSize < 100):
            jsonMergeFactorInput.write(str(outFullFileSize)[:2])
        elif (outFullFileSize >= 1 and outFullFileSize < 10):
            jsonMergeFactorInput.write(str(outFullFileSize)[:3])
        elif (outFullFileSize >= 0.1 and outFullFileSize < 1):
            jsonMergeFactorInput.write(str(outFullFileSize)[:3])
        elif (outFullFileSize >= 0.01 and outFullFileSize < 0.1):
            jsonMergeFactorInput.write(str(outFullFileSize)[:4])
        elif (outFullFileSize <= 0.01):
            jsonMergeFactorInput.write(str(outFullFileSize))

        if (bkg != 'taupairBGx0'):
            jsonTimeInput.write(',')
            jsonEvtSizeInput.write(',')
            jsonMergeFactorInput.write(',')
        elif (bkg == 'taupairBGx0'):
            jsonTimeInput.write(']')
            jsonEvtSizeInput.write(']')
            jsonMergeFactorInput.write(']')
    jsonTimeInput.write('\n')
    jsonEvtSizeInput.write('\n')
    jsonMergeFactorInput.write('\n')
jsonTimeInput.close()
jsonMergeFactorInput.close()
jsonEvtSizeInput.close()
