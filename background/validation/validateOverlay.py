#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>overlayPlots.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Runs BG overlay and makes validation histograms</description>
</header>
"""

import basf2 as b2
from simulation import add_simulation
import os
import glob
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed, TNtuple
import sys
import array

b2.set_random_seed(123452)


class Histogrammer(b2.Module):

    '''
    Make validation histograms for BG overlay.
    This validation assumes that all channels are active
    (no masked strip, dead pixels, ...)
    '''

    def initialize(self):
        ''' Initialize the Module: book histograms and set descriptions and checks'''

        #: list of digits histograms
        self.hist = []
        self.hist.append(TH1F('PXDDigits', 'PXDDigits (no data reduction)',
                              200, 10000, 80000))
        self.hist.append(TH1F('SVDShaperDigits', 'SVDShaperDigits', 100, 0, 8000))
        self.hist.append(TH1F('SVDShaperDigitsZS5', 'ZS5 SVDShaperDigits', 100, 0, 8000))
        self.hist.append(TH1F('CDCHits', 'CDCHits above threshold', 100, 0, 6000))
        self.hist.append(TH1F('TOPDigits', 'TOPDigits', 100, 0, 2000))
        self.hist.append(TH1F('ARICHDigits', 'ARICHDigits', 100, 0, 300))
        self.hist.append(TH1F('ECLDigits', 'ECLDigits, m_Amp > 500 (roughly 25 MeV)',
                              100, 0, 200))
        self.hist.append(TH1F('KLMDigits', 'KLMDigits', 150, 0, 150))

        #: list of occupancy-like histograms
        self.hist2 = []
        self.hist2.append(TH1F('PXDL1Occupancy', 'PXD L1 Occupancy', 100, 0, 4))
        self.hist2.append(TH1F('PXDL2Occupancy', 'PXD L2 Occupancy', 100, 0, 4))
        self.hist2.append(TH1F('PXDOccupancy', 'PXD Occupancy', 100, 0, 4))
        self.hist2.append(TH1F('SVDOccupancy', 'SVD L3 Occupancy', 100, 0, 15))
        self.hist2.append(TH1F('CDCInnerHitRate', 'CDC Hit Rate per wire for the Inner layers', 100, 0, 1000))
        self.hist2.append(TH1F('CDCOuterHitRate', 'CDC Hit Rate per wire for the Outer layers', 100, 0, 1000))
        self.hist2.append(TH1F('CDCHitRate', 'CDC Hit Rate per wire', 100, 0, 1000))
        self.hist2.append(TH1F("TOPHitTime", "TOP Hit Time Distribution; time [ns]", 300, -150, 150))
        #: numnber of events
        self.nTOPev = 0
        #: min TOP hit time
        self.minTOPTime = None
        #: max TOP hit time
        self.maxTOPTime = None

        #: number of PXD pixels (L1)
        self.nPXDL1 = 250*768*2 * 8
        #: number of PXD pixels (L2)
        self.nPXDL2 = 250*768*2 * 12
        if len(sys.argv) == 3 and int(sys.argv[2]) == 1003:
            self.nPXDL2 = 250*768*2 * 2
        #: number of PXD pixels (L1+L2)
        self.nPXD = self.nPXDL1 + self.nPXDL2

        #: number of L3 strips (u+v)
        self.nSVDL3 = 768 * 7 * 2 * 2

        #: number of inner CDC wires
        self.nCDCin = 160 * 8
        #: integration time of the inner CDC layers [ms]
        self.CDCITIn = 408.7 * 1e-6
        #: number of outer CDC wires
        self.nCDCout = 160*6 + 192*6 + 224*6 + 256*6 + 288*6 + 320*6 + 352*6 + 384 * 6
        #: integration time of the outer CDC layers [ms]
        self.CDCITout = 754.6 * 1e-6

        #: merged list of all histograms
        self.allh = self.hist+self.hist2

        for h in self.allh:
            if h.GetName() == 'PXDOccupancy':
                h.SetXTitle('PXD occupancy [%]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'PXD L1+L2 Occupancy per event - no data reduction (with BG overlay only and no event generator)')
            elif h.GetName() == 'PXDL1Occupancy':
                h.SetXTitle('PXD L1 occupancy [%]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'PXD L1 Occupancy per event - no data reduction (with BG overlay only and no event generator)')
            elif h.GetName() == 'PXDL2Occupancy':
                h.SetXTitle('PXD L2 occupancy [%]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'PXD L2 Occupancy per event - no data reduction (with BG overlay only and no event generator)')
            elif h.GetName() == 'SVDOccupancy':
                h.SetXTitle('SVD L3 ZS5 occupancy - average u,v [%]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'SVD ZS5 L3 Occupancy per event average u,v (with BG overlay only and no event generator)')
            elif h.GetName() == 'CDCHitRate':
                h.SetXTitle('CDC Hit Rate [kHz/wire]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'CDC Hit Rate, averaged inner/outer wires (with BG overlay only and no event generator)')
            elif h.GetName() == 'CDCInnerHitRate':
                h.SetXTitle('CDC Hit Rate for Inner Layers [kHz/wire]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'CDC Hit Rate per wire, for the Inner layers (with BG overlay only and no event generator)')
            elif h.GetName() == 'CDCOuterHitRate':
                h.SetXTitle('CDC Hit Rate for Outer Layers [kHz/wire]')
                option = 'shifter'
                descr = TNamed(
                    'Description',
                    'CDC Hit Rate per wire, for the Outer layers (with BG overlay only and no event generator)')
            else:
                h.SetXTitle('number of digits in event')
                option = 'shifter'
                descr = TNamed('Description', 'Number of background ' + h.GetName() +
                               ' per event (with BG overlay only and no event generator)')

            if h.GetName() == 'PXDDigits':
                option = 'expert'
            elif h.GetName() == 'SVDShaperDigits':
                option = 'expert'
            elif h.GetName() == 'SVDShaperDigitsZS5':
                option = 'expert'
            elif h.GetName() == 'CDCHits':
                option = 'expert'
            elif h.GetName() == 'TOPHitTime':
                option = 'expert'

            h.SetYTitle('entries per bin')
            h.GetListOfFunctions().Add(descr)
            check = TNamed('Check', 'Distribution must agree with its reference')
            h.GetListOfFunctions().Add(check)
            contact = TNamed('Contact', 'marko.staric@ijs.si')
            h.GetListOfFunctions().Add(contact)
            options = TNamed('MetaOptions', option)
            h.GetListOfFunctions().Add(options)

            ntplvar = "PXDL1_occupancy:PXDL2_occupancy:PXD_occupancy:SVDL3ZS5_occupancy"
            ntplvar += ":CDCInner_hitRate:CDCOuter_hitRate:CDC_hitRate:TOP_hitRate"

            #: ntuple to store background levels
            self.ntpl = TNtuple("ntpl", "Average Background Levels", (ntplvar))

    def event(self):
        ''' Event processor: fill histograms '''

        for h in self.hist:
            digits = Belle2.PyStoreArray(h.GetName())
            if h.GetName() == 'CDCHits':
                continue
            if h.GetName() == 'ECLDigits':
                n = 0
                for digit in digits:
                    if digit.getAmp() > 500:  # roughly 25 MeV
                        n += 1
                h.Fill(n)
            else:
                h.Fill(digits.getEntries())

        # PXD
        pxdDigits = Belle2.PyStoreArray('PXDDigits')
        self.hist2[2].Fill(pxdDigits.getEntries()/self.nPXD * 100)
        nL1 = 0
        nL2 = 0
        for pxdDigit in pxdDigits:
            if pxdDigit.getSensorID().getLayerNumber() == 1:  # check L1/L2
                nL1 += 1
            else:
                nL2 += 1
        self.hist2[0].Fill(nL1/self.nPXDL1 * 100)
        self.hist2[1].Fill(nL2/self.nPXDL2 * 100)

        # SVD
        svdDigits = Belle2.PyStoreArray('SVDShaperDigitsZS5')
        nL3 = 0
        for svdDigit in svdDigits:
            if svdDigit.getSensorID().getLayerNumber() == 3:  # only check SVD L3
                nL3 += 1
        self.hist2[3].Fill(nL3/self.nSVDL3 * 100)

        # CDC
        cdcHits = Belle2.PyStoreArray('CDCHits')
        nCDC_in = 0
        nCDC_out = 0
        for cdcHit in cdcHits:  # count wires of inner/outer layers
            if cdcHit.getISuperLayer() == 0 and cdcHit.getADCCount() > 15:
                nCDC_in += 1
            if cdcHit.getISuperLayer() != 0 and cdcHit.getADCCount() > 18:
                nCDC_out += 1
        self.hist2[4].Fill(nCDC_in/self.nCDCin / self.CDCITIn)
        self.hist2[5].Fill(nCDC_out/self.nCDCout / self.CDCITout)
        self.hist2[6].Fill((nCDC_in+nCDC_out) / (self.nCDCin * self.CDCITIn + self.nCDCout * self.CDCITout))
        self.hist[3].Fill(nCDC_in+nCDC_out)

        # TOP
        self.nTOPev += 1
        for TOPDigit in Belle2.PyStoreArray('TOPDigits'):
            if TOPDigit.getHitQuality() != Belle2.TOPDigit.c_Good:
                continue
            self.hist2[7].Fill(TOPDigit.getTime())
            if self.minTOPTime:
                self.minTOPTime = min(self.minTOPTime, TOPDigit.getTime())
                self.maxTOPTime = max(self.maxTOPTime, TOPDigit.getTime())
            else:
                self.minTOPTime = TOPDigit.getTime()
                self.maxTOPTime = TOPDigit.getTime()

    def terminate(self):
        """ Write histograms to file."""

        TOPbg = self.hist2[7].Integral(101, 150)/self.nTOPev/50e-3/16/32

        # ntpl content "PXDL1:PXDL2:PXD:SVDL3ZS5:CDCInner:CDCOuter:CDC:TOP");
        values = [self.hist2[0].GetMean(), self.hist2[1].GetMean(), self.hist2[2].GetMean(),
                  self.hist2[3].GetMean(), self.hist2[4].GetMean(), self.hist2[5].GetMean(),
                  self.hist2[6].GetMean(), float(TOPbg)]

        self.ntpl.Fill(array.array("f", values),)
        tfile = TFile('overlayPlots.root', 'recreate')
        self.ntpl.Write()
        for h in self.allh:
            h.Write()
        tfile.Close()


bg = None
exp = 0
if len(sys.argv) == 3:
    b2.B2INFO('Using background samples for experiment ' + sys.argv[2] + ' at ' + sys.argv[1])
    bg = glob.glob(str(sys.argv[1]) + '/*.root')
    exp = int(sys.argv[2])
elif 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    if bg is None:
        b2.B2FATAL('No beam background samples found in folder ' +
                   os.environ['BELLE2_BACKGROUND_DIR'])
    b2.B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_DIR'])
else:
    b2.B2FATAL('variable BELLE2_BACKGROUND_DIR is not set')

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1], 'expList': [exp]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True, usePXDDataReduction=False, forceSetPXDDataReduction=True,
               simulateT0jitter=False)

# Add SVD offline zero-suppression (ZS5)
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)

# Make histograms
main.add_module(Histogrammer())

# Show progress of processing
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
