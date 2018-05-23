#####################################################################################
#
#
#
#
#
#  This module prints out the preformance of the different skimming scripts
#
#        By Racha Cheaib March 1st, 2017
################################################################################

# !/usr/bin/env python3
from basf2 import *
import os
import itertools
import sys
import collections
from skimExpertFunctions import *

skims = 'BottomoniumEtabExclusive'

#  skimNames1 = ' BtoDh_Kspi0 BtoDh_Kspipipi0 BtoDh_Kshh BtoDh_hh BtoPi0Pi0
# BottomoniumEtabExclusive BottomoniumUpsilon  SLUntagged LeptonicUntagged
# skimName2 = ' Charm2BodyHadronic Charm3BodyHadronic Charm3BodyHadronic2
# Charm2BodyHadronicD0 CharmSemileptonic  Charm2BodyNeutrals
# Charm2BodyNeutralsD0 CharmRare CharmlessHad SystematicsJpsiee'
# skiName3 = ' SystematicsJpsimumu SystematicsDstar Tau TCPV
#  PRsemileptonicUntagged DoubleCharm feiHadronicB0 feiHadronicBplus
# feiSLB0WithOneLep feiSLBplusWithOneLep '

bkgs = ' mixedBGx1  chargedBGx1 ccbarBGx1 ssbarBGx1 uubarBGx0  ddbarBGx1  taupairBGx1 '
# mixedBGx0 chargedBGx0 ccbarBGx0 ssbarBGx0 uubarBGx0 ddbarBGx0 taupairBGx0

jsonMergeFactorInput = open('JsonMergeFactorInput.txt', 'w')
jsonEvtSizeInput = open('JsonEvtSizeInput.txt', 'w')
jsonTimeInput = open('JsonTimeInput.txt', 'w')

nFullFiles = 1000
nFullEvents = 200000

for skim in skims.split():
    jsonTimeInput.write('t_' + skim + '=[')
    jsonEvtSizeInput.write('s_' + skim + '=[')
    jsonMergeFactorInput.write('m_' + skim + '=[')
    skimCode = getOutputLFN(skim)
    print('|Skim:' + skim + '_Skim_Standalone Statistics|')
    title = '|Bkg        |     Retention   | Time/Evt(HEPSEC)| Total Time (s) |uDSTSize/Evt(KB)|'
    title += ' uDSTSize(MB)|  ACMPE   |Log Size/evt(KB)|Log Size(MB)|'
    title += ' AvgMemory/Evt(KB)|MaxMemory/Evt(KB)| FullSkimSize(GB)|'
    title += ' FullSkimLogSize(GB)|'
    print(title)
    for bkg in bkgs.split():
        inputFileName = 'outputFiles/' + skim + '_' + bkg + '.out'
        outputFileName = 'outputFiles/' + skim + '_' + bkg
        outputUdstName = 'outputFiles/' + skimCode + '_' + bkg
        outputMdstName = 'outputMdstFiles/' + skimCode + '_' + bkg

        if (bkg == 'mixedBGx1'):
            nFullEvents = 120000
            nFullFiles = 3564
        if (bkg == 'chargededBGx1'):
            nFullEvents = 120000
            nFullFiles = 3770
        if (bkg == 'uubarBGx1'):
            nFullEvents = 210000
            nFullFiles = 6115
        if (bkg == 'ddbarBGx1'):
            nFullEvents = 180000
            nFullFiles = 1783
        if (bkg == 'ssbarBGx1'):
            nFullEvents = 180000
            nFullFiles = 1704
        if (bkg == 'ccbarBGx1'):
            nFullEvents = 150000
            nFullFiles = 7088
        if (bkg == 'taupairBGx1'):
            nFullEvents = 210000
            nFullFiles = 3501

        if (bkg == 'mixedBGx0'):
            nFullEvents = 300000
            nFullFiles = 357
        if (bkg == 'chargededBGx0'):
            nFullEvents = 300000
            nFullFiles = 377
        if (bkg == 'uubarBGx0'):
            nFullEvents = 400000
            nFullFiles = 803
        if (bkg == 'ddbarBGx0'):
            nFullEvents = 400000
            nFullFiles = 201
        if (bkg == 'ssbarBGx0'):
            nFullEvents = 400000
            nFullFiles = 192
        if (bkg == 'ccbarBGx0'):
            nFullEvents = 350000
            nFullFiles = 760
        if (bkg == 'taupairBGx0'):
            nFullEvents = 500000
            nFullFiles = 368

        n = 0
        l = 0
        a = 0
        z = 0
        s = 0
        f = 0
        acmCounter = 0
        acmN = 0
        acmD = 0
        time = 0.0
        events = 10000
        nModes = 0
        skipped = False
        timePerEvent = 0
        retention = 0
        mdstSizeByte = 0
        mdstSizeKiloByte = 0
        mdstSizePerEvent = 0
        udstSizeByte = 0
        udstSizePerEvent = 0
        udstSizePerEvent = 0
        totalTime = 0
        maxMemory = 0
        diff = 1
        with open(inputFileName, 'r') as inF:
            content = []
            for line in inF:
                n += 1
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
                    l += 1  # print(line)
                    rline = line.split(' ')
                    retention = float(rline[2])  # print('Total retention is ' + str(retention))
                if z == 0 and line.find('Candidate Multiplicity') >= 0:  # Find line with Candidate Multiplicity
                    z += 1
                    s = int(n) + 2
                if a == 0 and line.find('Total Retention') >= 0:
                    a += 1
                    f = int(n) - 3

# Candidate multiplicity is printed out per decay mode, We need the average so
# we count how many modes (i.e. lines) and then we use that as a
# denominator to determine the average candidate multiplicity
        if (s != 0 and f != 0):
            diff = f - s
        for i in range(0, diff):
            sline = content[s + i].split()
            if (i < 10):
                acmTemp = '1'  # sline[4]
                if any("INFO" or "ARNING" or "WARNING" in strip for strip in sline):
                    # print('warning')#<-----------------PRINT THIS OUT TO SEE WHEN YOU HAVE A STUPID WARNING FOR NO APPARENT REASON
                    nModes = diff - i
                    skipped = True
                    break
                if acmTemp[:1].isdigit():
                    acmN = acmN + float(sline[4])
                else:
                    acmN = acmN + 0
                    nModes += 1
                    skipped = True
            elif (i > 9):
                acmTemp = sline[3]
                if any("INFO" or "ARNING" or "WARNING" in strip for strip in sline):
                    # print('warning')#<-----------------PRINT THIS OUT TO SEE WHEN YOU HAVE A STUPID WARNING FOR NO APPARENT REASON
                    nModes = diff - i
                    skipped = True
                    break
                elif acmTemp[:1].isdigit():
                    acmN = acmN + float(sline[3])
                else:
                    acmN = acmN + 0
                    nModes = +1
                    skipped = True

        acm = acmN / diff
        # GET AND PRINT UDST EVENT SIZE:
        if retention == 0:
            udstSizePerEvent = 0
        if retention == 0:
            mdstSizePerEvent = 0

        statinfo_mdst = os.stat(outputUdstName + '.udst.root')
        mdstSizeByte = str(statinfo_mdst.st_size)
        mdstSizeKiloByte = statinfo_mdst.st_size / 1024
        mdstSizePerEvent = mdstSizeKiloByte / events

        statinfo_udst = os.stat(outputUdstName + '.udst.root')
        udstSizeByte = str(statinfo_udst.st_size)
        udstSizeKiloByte = statinfo_udst.st_size / 1024

        statinfo_log = os.stat(outputFileName + '.out')
        logFileSizeByte = str(statinfo_log.st_size)
        logFileSizeKiloByte = statinfo_log.st_size / 1024
        logFileSizePerEvent = logFileSizeKiloByte / events

        if (retention != 0):
            udstSizePerEvent = udstSizeKiloByte / (events * retention)
        if (retention != 0):
            mdstSizePerEvent = mdstSizeKiloByte / (events * retention)
        fullFileSizeKB = udstSizePerEvent * retention * nFullEvents
        fullFileSizeMB = fullFileSizeKB / 1000

        fullSkimSizeMB = fullFileSizeMB * nFullFiles
        fullSkimSizeGB = fullSkimSizeMB / 1000
        fullLogFileMB = logFileSizeKiloByte / 1000
        fullLogSkimSizeGB = fullLogFileMB * nFullFiles / 1000

        totalTime = timePerEvent * nFullEvents / 23.57
        maxMemory = maxMemoryPerEvent * nFullEvents / 1000000

        print('|' +
              bkg +
              '     |     ' +
              str(retention) +
              '     |     ' +
              str(timePerEvent)[:5] +
              '      |     ' +

              str(totalTime)[:5] +
              '      |     ' +
              str(udstSizePerEvent)[:5] +
              '     |     ' +
              str(fullFileSizeMB)[:5] +
              '    |   ' +
              str(acm)[:5] +
              '     |     ' +
              str(logFileSizePerEvent)[:5] +
              '     |     ' +
              str(fullLogFileMB)[:5] +
              '     |    ' +
              str(maxMemoryPerEvent)[:5] +
              '     |      ' +
              str(avgMemoryPerEvent)[:5] +
              '      |      ' +
              str(fullSkimSizeGB)[:5] +
              '   |    ' +
              str(fullLogSkimSizeGB)[:5] +
              "\n")
        # if skipped:#<-----PRINT THIS OUT TO KNOW HOW ACCURATE ACMPE IS
        # print('Skipped '+str(nModes)+' in the calculation of ACMPE')
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
