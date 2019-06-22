#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

import sys
import math
import ROOT
from array import array
from ROOT import Belle2
from ROOT.Belle2 import TOP
from ROOT import TCanvas, TH1F, TFile
import pandas as pd


class PrintTTSPar(Module):

    def terminate(self):

        print("creating the LUT")
        # first, create a LUT that allows to go form PMT ID and PMTpixelID to hwchannel and pixel Row/Col
        # the index of these arrays is a global identifier for the PMT ID/pixel constructed as
        # index = pixel-1 + 16*(PMT-1)
        mapper = TOP.TOPGeometryPar.Instance().getChannelMapper()
        hwChannels = [None] * 512
        pmtIDs = [None] * 512
        pmtPixels = [None] * 512
        pixelRows = [None] * 512
        pixelCols = [None] * 512
        for ch in range(512):
            m_pixelID = mapper.getPixelID(ch)

            pixelRow = int((m_pixelID - 1) / 64 + 1)
            pixelCol = int((m_pixelID - 1) % 64 + 1)

            pmtCol = int((pixelCol - 1) / 4 + 1)
            pmtRow = int((pixelRow - 1) / 4 + 1)

            pmtPixelRow = (pixelRow - 1) % 4 + 1
            pmtPixelCol = (pixelCol - 1) % 4 + 1

            pmtID = pmtCol + (pmtRow - 1) * 16
            pmtPixel = pmtPixelCol + (pmtPixelRow - 1) * 4

            index = pmtPixel - 1 + 16 * (pmtID - 1)
            pixelRows[index] = pixelRow
            pixelCols[index] = pixelCol
            pmtIDs[index] = pmtID
            pmtPixels[index] = pmtPixel
            hwChannels[index] = ch

        print("LUT ready")

        channels = [None] * 8192
        slots = [None] * 8192
        pmtIDs = [None] * 8192
        pmtPixelIDs = [None] * 8192
        rows = [None] * 8192
        cols = [None] * 8192
        nGauss = [None] * 8192
        m1s = [None] * 8192
        m2s = [None] * 8192
        m3s = [None] * 8192
        s1s = [None] * 8192
        s2s = [None] * 8192
        s3s = [None] * 8192
        f1s = [None] * 8192
        f2s = [None] * 8192
        f3s = [None] * 8192

        print("Dumping all the TTS info into arrays")
        # sort installation data according to slot number and pmtID
        pmtInstal_sorted = {}
        for pmt in Belle2.PyDBArray('TOPPmtInstallations'):
            slot = pmt.getSlotNumber()
            if slot not in pmtInstal_sorted:
                pmtInstal_sorted[slot] = {}
            pmtID = pmt.getPosition()
            if pmtID not in pmtInstal_sorted[slot]:
                pmtInstal_sorted[slot][pmtID] = pmt

        # sort TTS parameters according to PMT serial number
        pmtPar_sorted = {}
        for tts in Belle2.PyDBArray('TOPPmtTTSPars'):
            sn = tts.getSerialNumber()
            if sn not in pmtPar_sorted:
                pmtPar_sorted[sn] = tts

        # get TTS par for all slots and PMT's:
        for slot in pmtInstal_sorted:
            for pmtID in pmtInstal_sorted[slot]:
                sn = pmtInstal_sorted[slot][pmtID].getSerialNumber()
                if sn in pmtPar_sorted:
                    tts = pmtPar_sorted[sn]
                    for pmtPixel in range(1, 17):
                        ch = hwChannels[pmtPixel - 1 + 16 * (pmtID - 1)]
                        gch = ch + 512 * (slot - 1)
                        slots[gch] = slot
                        channels[gch] = ch
                        pmtIDs[gch] = pmtID
                        pmtPixelIDs[gch] = pmtPixel
                        rows[gch] = pixelRows[pmtPixel - 1 + 16 * (pmtID - 1)]
                        cols[gch] = pixelCols[pmtPixel - 1 + 16 * (pmtID - 1)]
                        nGauss[gch] = tts.getGaussians(pmtPixel).size()

                        m1s[gch] = tts.getGaussians(pmtPixel)[0].mean
                        s1s[gch] = tts.getGaussians(pmtPixel)[0].sigma
                        f1s[gch] = tts.getGaussians(pmtPixel)[0].fraction

                        m2s[gch] = tts.getGaussians(pmtPixel)[1].mean
                        s2s[gch] = tts.getGaussians(pmtPixel)[1].sigma
                        f2s[gch] = tts.getGaussians(pmtPixel)[1].fraction

                        if tts.getGaussians(pmtPixel).size() > 2:
                            m3s[gch] = tts.getGaussians(pmtPixel)[2].mean
                            s3s[gch] = tts.getGaussians(pmtPixel)[2].sigma
                            f3s[gch] = tts.getGaussians(pmtPixel)[2].fraction
                        else:
                            m3s[gch] = 0.
                            s3s[gch] = 0.
                            f3s[gch] = 0.
                else:
                    B2ERROR('serial number ' + sn + ' not found in TOPPmtTTSPars')

        # Now we have all the infos we need, but inconveniently sorted by PMT number
        # instead of hw channel, as we do when we collect the laser data
        # So, let's sort them before saving them in a  TTree
        '''
        print("Sorting the TTS info array according to the global channel number")

        channels_s = [x for _,x in sorted(zip(channels, globalChannels))]
        slots_s = [x for _,x in sorted(zip(slots, globalChannels))]
        pmtIDs_s  = [x for _,x in sorted(zip(pmtIDs, globalChannels))]
        pmtPixelIDs_s = [x for _,x in sorted(zip(pmtPixelIDs, globalChannels))]
        nGauss_s  = [x for _,x in sorted(zip(nGauss, globalChannels))]
        rows_s  = [x for _,x in sorted(zip(rows, globalChannels))]
        cols_s  = [x for _,x in sorted(zip(cols, globalChannels))]
        m1s_s = [x for _,x in sorted(zip(m1s, globalChannels))]
        m2s_s = [x for _,x in sorted(zip(m2s, globalChannels))]
        m3s_s = [x for _,x in sorted(zip(m3s, globalChannels))]
        s1s_s = [x for _,x in sorted(zip(s1s, globalChannels))]
        s2s_s = [x for _,x in sorted(zip(s2s, globalChannels))]
        s3s_s = [x for _,x in sorted(zip(s3s, globalChannels))]
        f1s_s = [x for _,x in sorted(zip(f1s, globalChannels))]
        f2s_s = [x for _,x in sorted(zip(f2s, globalChannels))]
        f3s_s = [x for _,x in sorted(zip(f3s, globalChannels))]
        '''
        print("Saving the info into a root tree")

        tfile = ROOT.TFile("TTSParametrizations.root", 'recreate')
        tree = ROOT.TTree('tree', 'TTS parametrizations')

        slot = array('h', [0])
        channel = array('h', [0])
        pmtID = array('h', [0])
        pmtPixel = array('h', [0])
        pixelRow = array('h', [0])
        pixelCol = array('h', [0])

        mean1 = array('f', [0])
        mean2 = array('f', [0])
        mean3 = array('f', [0])
        sigma1 = array('f', [0])
        sigma2 = array('f', [0])
        sigma3 = array('f', [0])
        fraction1 = array('f', [0])
        fraction2 = array('f', [0])
        fraction3 = array('f', [0])

        tree.Branch('slot', slot, 'slot/S')
        tree.Branch('channel', channel, 'channel/S')
        tree.Branch('pmtID', pmtID, 'pmtID/S')
        tree.Branch('pmtPixel', pmtPixel, 'pmtPixel/S')
        tree.Branch('pixelRow', pixelRow, 'pixelRow/S')
        tree.Branch('pixelCol', pixelCol, 'pixelCol/S')

        tree.Branch('mean1', mean1, 'mean1/F')
        tree.Branch('mean2', mean2, 'mean2/F')
        tree.Branch('mean3', mean3, 'mean3/F')

        tree.Branch('sigma1', sigma1, 'sigma1/F')
        tree.Branch('sigma2', sigma2, 'sigma2/F')
        tree.Branch('sigma3', sigma3, 'sigma3/F')

        tree.Branch('fraction1', fraction1, 'fraction1/F')
        tree.Branch('fraction2', fraction2, 'fraction2/F')
        tree.Branch('fraction3', fraction3, 'fraction3/F')

        for globalCh in range(8192):
            slot[0] = slots[globalCh]
            channel[0] = channels[globalCh]
            pmtID[0] = pmtIDs[globalCh]
            pmtPixel[0] = pmtPixelIDs[globalCh]
            pixelRow[0] = rows[globalCh]
            pixelCol[0] = cols[globalCh]
            mean1[0] = m1s[globalCh]
            mean2[0] = m2s[globalCh]
            mean3[0] = m3s[globalCh]
            sigma1[0] = s1s[globalCh]
            sigma2[0] = s2s[globalCh]
            sigma3[0] = s3s[globalCh]
            fraction1[0] = f1s[globalCh]
            fraction2[0] = f2s[globalCh]
            fraction3[0] = f3s[globalCh]
            tree.Fill()
        tree.Write()
        tfile.Close()
        print("Bye bye!")


use_central_database('data_reprocessing_proc8')

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', True)

main.add_module(geometry)


# print from DB
main.add_module(PrintTTSPar())

# Process events
process(main)
