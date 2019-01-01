
# !/usr/bin/env python3


"""
This is a tool that prints out the follwing information per skim for specified MC sample types:
   1) Total Retention Rate
   2) Total Time/Event  (HEPSEC)
   3) Total uDST size /Event (kB)

   1) Average Retention Rate
   2) Average Time/Event  (HEPSEC)
   3) Average uDST size /Event (kB)



To run printAvgPerSkim.py, you need to have run your skim on a set of input mDST files
(you can use runSkims.py), and produce a set of uDST and log files with the following
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

skims = ' ALP3Gamma BottomoniumEtabExclusive BottomoniumUpsilon TauGeneric SystematicsRadMuMu SystematicsRadEE'
skims += ' LFVZpInvisible LFVZpVisible SinglePhotonDark SystematicsTracking'
skims += '  SystematicsLambda  Systematics ISRpipicc BtoDh_Kspipipi0 BtoPi0Pi0 '
skims += 'feiSLB0WithOneLep  feiHadronicB0 feiHadronicBplus '
skims += '  BtoDh_Kspi0  BtoDh_hh TauGeneric  PRsemileptonicUntagged SLUntagged LeptonicUntagged TCPV'
skims += ' CharmRare BtoXll BtoXgamma  TauLFV Charm3BodyHadroni3 Charm3BodyHadronic'
skims += ' Charm3BodyHadronic1 Charm3BodyHadronic2    Charm2BodyNeutrals Charm2BodyNeutralsD0'

bkgs = 'MC9_mixedBGx1  MC9_chargedBGx1 MC9_ccbarBGx1 MC9_ssbarBGx1 MC9_uubarBGx0  MC9_ddbarBGx1  MC9_taupairBGx1'
bkgs += ' MC9_mixedBGx0 MC9_chargedBGx0 MC9_ccbarBGx0 MC9_ssbarBGx0 MC9_uubarBGx0 MC9_ddbarBGx0 MC9_taupairBGx0'


nFullEvents = 200000
nBkgs = 14


totalUdstSizePerEventPerSkim = 0
totalRetentionPerSkim = 0
totalProcessingTimePerEventPerSkim = 0

avgUdstSizePerEventPerSkim = 0
avgRetentionPerSkim = 0
avgProcessingTimePerEventPerSkim = 0
release = 'prerelease-03-00-0b'
statsFile = open('Stats.txt', 'w')
retentionFile = open('Ret_' + release + '.txt', 'w+')
evtSizeFile = open('EvtSize_' + release + '.txt', 'w+')
timeFile = open('ProcessingTime_' + release + '.txt', 'w+')


skimList = "vector<TString> skims ={"
retList = "vector<Double_t> retention={"
sizeList = "vector<Double_t> evtSize={"
timeList = "vector<Double_t> time={"

for skim in skims.split():
    print('|skim:' + skim)
    title = '|Skim       |  Total   Retention   | Total Time/Evt(HEPSEC)| Total uDSTSize/Evt(KB)|'

    title += '  Average   Retention   | Average Time/Evt(HEPSEC)| Average uDSTSize/Evt(KB)|'
    print(title)

    skimList += '\"' + skim + '\"' + ','

    avgUdstSizePerEventPerSkim = 0
    avgRetentionPerSkim = 0
    avgProcessingTimePerEventPerSkim = 0

    totalUdstSizePerEventPerSkim = 0
    totalRetentionPerSkim = 0
    totalProcessingTimePerEventPerSkim = 0
    for bkg in bkgs.split():
        inputFileName = skim + '_' + bkg + '.out'
        outputFileName = skim + '_' + bkg
        outputUdstName = skim + '_' + bkg
        pos = bkg.find('_')
        skimCampaign = bkg[0:pos]
        sampleType = bkg[pos + 1:]
        fileList = get_test_file(sampleType, skimCampaign)
        nFullEvents = get_eventN(fileList)
        nSkimmedEvents = get_eventN(outputUdstName + '.udst.root')
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
        totalRetentionPerSkim += retention
        totalUdstSizePerEventPerSkim += udstSizePerEvent
        totalProcessingTimePerEventPerSkim += timePerEvent
        print("Retention for this skim and sample :" + str(retention) +
              " which makes total retention " + str(totalRetentionPerSkim))
    print('Average calculated over ' + str(nBkgs) + ' samples')
    avgRetentionPerSkim = totalRetentionPerSkim / nBkgs
    avgUdstSizePerEventPerSkim = totalUdstSizePerEventPerSkim / nBkgs
    avgProcessingTimePerEventPerSkim = totalProcessingTimePerEventPerSkim / nBkgs

    result = '|' + skim + '    |  ' + str(totalRetentionPerSkim)[:8] + ' | ' + \
        str(totalProcessingTimePerEventPerSkim)[:8] + '  |  ' + str(totalUdstSizePerEventPerSkim)[:8] + '|'

    result += ' ' + str(avgRetentionPerSkim)[:8] + ' | ' + str(avgProcessingTimePerEventPerSkim)[:8] + \
        '  |  ' + str(avgUdstSizePerEventPerSkim)[:8] + '|'
    print(result)
    retList += str(avgRetentionPerSkim)[:8]
    retList += (',')
    sizeList += (str(avgUdstSizePerEventPerSkim)[:8])
    sizeList += (',')
    timeList += (str(avgProcessingTimePerEventPerSkim)[:8])
    timeList += (',')
    statsFile.write(result)

retList += ('}')
sizeList += ('}')
timeList += ('}')


statsFile.write("\n")
statsFile.write(retList)
statsFile.write("\n")
statsFile.write(sizeList)
statsFile.write("\n")
statsFile.write(timeList)


statsFile.close()
