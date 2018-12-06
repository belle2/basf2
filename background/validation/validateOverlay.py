#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>overlayPlots.root</output>
  <contact>staric</contact>
  <description>Runs BG overlay and makes validation histograms</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
import glob
import sys
import math
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed

set_random_seed(123452)


class Histogrammer(Module):

    '''
    Make validation histograms for BG overlay.
    '''

    def initialize(self):
        ''' Initialize the Module: book histograms and set descriptions and checks'''

        #: list of histograms
        self.hist = []
        self.hist.append(TH1F('PXDDigits', 'PXDDigits (no data reduction)',
                              100, 30000, 40000))
        self.hist.append(TH1F('SVDShaperDigits', 'SVDShaperDigits', 100, 0, 2000))
        self.hist.append(TH1F('CDCHits', 'CDCHits', 100, 1000, 2000))
        self.hist.append(TH1F('TOPDigits', 'TOPDigits', 100, 0, 700))
        self.hist.append(TH1F('ARICHDigits', 'ARICHDigits', 100, 0, 300))
        self.hist.append(TH1F('ECLDigits', 'ECLDigits, m_Amp > 500 (roughly 25 MeV)',
                              100, 0, 100))
        self.hist.append(TH1F('BKLMDigits', 'BKLMDigits', 150, 0, 150))
        self.hist.append(TH1F('EKLMDigits', 'EKLMDigits', 100, 0, 100))

        for h in self.hist:
            h.SetXTitle('number of digits in event')
            h.SetYTitle('entries per bin')
            descr = TNamed('Description', 'Number of background ' + h.GetName() +
                           ' per event (with BG overlay only and no event generator)')
            h.GetListOfFunctions().Add(descr)
            check = TNamed('Check', 'Distribution must agree with its reference')
            h.GetListOfFunctions().Add(check)
            contact = TNamed('Contact', 'marko.staric@ijs.si')
            h.GetListOfFunctions().Add(contact)

    def event(self):
        ''' Event processor: fill histograms '''

        for h in self.hist:
            digits = Belle2.PyStoreArray(h.GetName())
            if h.GetName() == 'ECLDigits':
                n = 0
                for digit in digits:
                    if digit.getAmp() > 500:  # roughly 25 MeV
                        n += 1
                h.Fill(n)
            else:
                h.Fill(digits.getEntries())

    def terminate(self):
        """ Write histograms to file."""

        tfile = TFile('overlayPlots.root', 'recreate')
        for h in self.hist:
            h.Write()
        tfile.Close()


bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    if bg is None:
        B2FATAL('No beam background samples found in folder ' +
                os.environ['BELLE2_BACKGROUND_DIR'])
    B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_DIR'])
else:
    B2FATAL('variable BELLE2_BACKGROUND_DIR is not set')

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True, usePXDDataReduction=False)

# Make histograms
main.add_module(Histogrammer())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
