#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# convert an output root file of pixel-by-pixel gain analysis data to pmt-by-pmt format
# Contibutors: Maeda Yosuke (KMI, Nagoya Univ.)
#
# Usage: basf2 gainAnalysisPixelToPMT.py inputRoot(pixel-by-pixel result) outputRoot [maxHitTiming=-8.] [nMinEntries=100]
# ---------------------------------------------------------------------------------------

import sys
import time
import numpy
from array import array
from basf2 import *
from ROOT import gROOT
gROOT.SetBatch(True)
from ROOT import TFile, TTree, TMath


def gainana_pixelToPmt(inputRoot, outputRoot, maxHitTiming=-8., nMinEntries=100):

    f = TFile(inputRoot)
    tr = f.Get("tree")

    f_out = TFile(outputRoot, "RECREATE")
    tr_out = TTree("tree_pmt", "gain/eff. analysis output in PMT-by-PMT")

    nPmtCh = 16
    nTopModules = 16
    nPmtPerModule = 32
    slotId = array('i', [0])
    pmtId = array('i', [0])
    nValidCh = array('i', [0])
    meanGain = array('f', [0.])
    stdDevGain = array('f', [0.])
    maxMinRatio = array('f', [0.])
    gain = array('f', nPmtCh * [0.])
    efficiency = array('f', nPmtCh * [0.])

    tr_out.Branch('slotId', slotId, 'slotId/I')
    tr_out.Branch('pmtId', pmtId, 'pmtId/I')
    tr_out.Branch('nValidCh', nValidCh, 'nValidCh/I')
    tr_out.Branch('meanGain', meanGain, 'meanGain/F')
    tr_out.Branch('stdDevGain', stdDevGain, 'stdDevGain/F')
    tr_out.Branch('maxMinRatio', maxMinRatio, 'maxMinRatio/F')
    tr_out.Branch('gain', gain, 'gain[' + str(nPmtCh) + ']/F')
    tr_out.Branch('efficiency', efficiency, 'efficiency[' + str(nPmtCh) + ']/F')

    for iSlot in range(nTopModules):
        for iPMT in range(nPmtPerModule):

            slotId[0] = iSlot + 1
            pmtId[0] = iPMT + 1

            cut = "slotId==" + str(slotId[0]) + " && pmtId==" + str(pmtId[0]) + \
                " && hitTiming<(" + str(maxHitTiming) + ") && nEntries>" + str(nMinEntries)
            tr.Draw("gain:efficiency:pmtChId", cut)
            nValidCh[0] = tr.GetEntries(cut)
            if nValidCh[0] < 2:
                continue
            elif nValidCh[0] >= nPmtCh:
                print("ERROR : too many channels for slot" + str(slotId[0]).zfill(2) +
                      " PMT" + str(pmtId[0]).zfill(2) + "(" + str(nValidCh[0]) + ")")

            gainArray = []
            effArray = []
            chArray = []
            for iCh in range(nValidCh[0]):
                gainArray.append(tr.GetV1()[iCh])
                effArray.append(tr.GetV2()[iCh])
                chArray.append(int(tr.GetV3()[iCh]))

            meanGain[0] = numpy.average(gainArray)
            stdDevGain[0] = numpy.std(gainArray)
            maxMinRatio[0] = max(gainArray) / min(gainArray)
            print(cut + " : nValidCh = " + str(nValidCh[0]))
            print(" --> mean = " + str('%05.2f' % meanGain[0]) + ", StdDev = " + str('%05.2f' % stdDevGain[0]) +
                  ", max/min = " + str('%03.2f' % maxMinRatio[0]))

            for iCh in range(nPmtCh):
                for jCh in range(nValidCh[0]):
                    if chArray[jCh] - 1 is iCh:
                        gain[iCh] = gainArray[jCh]
                        efficiency[iCh] = effArray[jCh]
                        break
                    else:
                        gain[iCh] = -1
                        efficiency[iCh] = -1

            tr_out.Fill()

    f_out.cd()
    tr_out.Write()
    f_out.Close()

if __name__ == '__main__':

    args = sys.argv
    if len(args) > 4:
        gainana_pixelToPmt(args[1], args[2], float(args[3]), int(args[4]))
    elif len(args) > 3:
        gainana_pixelToPmt(args[1], args[2], float(args[3]))
    elif len(args) > 2:
        gainana_pixelToPmt(args[1], args[2])
    else:
        print("usage:")
        print(args[0] + " (root file for pixel-by-pixel gain result) (output file)" +
              " [max. hit timing = -8 ns] [min. number of entries = 100]")
