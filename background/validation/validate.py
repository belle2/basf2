#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>timeDistributions.root</output>
  <contact>staric</contact>
  <description>Runs BG mixer and makes validation histograms</description>
</header>
"""

from basf2 import *
import glob
import math
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed

set_random_seed(123452)


class BGHistogrammer(Module):

    '''
    Make validation histograms for BG mixer.
    '''

    #: list of SimHits to be histogramed
    simHits = [
        'PXDSimHits',
        'SVDSimHits',
        'CDCSimHits',
        'TOPSimHits',
        'ARICHSimHits',
        'ECLHits',
        'BKLMSimHits',
        'EKLMSimHits',
    ]
    #: length of simHits list
    n = len(simHits)

    def initialize(self):
        ''' Initialize the Module: set histogram axis titles, description and check'''

        #: list of histograms
        self.hist = [TH1F('h' + str(i), self.simHits[i], 400, -20, 20) for i in range(self.n)]

        for i in range(self.n):
            self.hist[i].GetXaxis().SetTitle('time [#mus]')
            self.hist[i].GetYaxis().SetTitle('entries/bin')
            descr = TNamed('Description', 'Time distribution of ' + self.simHits[i] +
                           ' for mixed background')
            self.hist[i].GetListOfFunctions().Add(descr)
            check = TNamed('Check', 'Distribution must be flat in the time window. ' +
                           'Bin statistics is not Poissonian.')
            self.hist[i].GetListOfFunctions().Add(check)
            contact = TNamed('Contact', 'marko.staric@ijs.si')
            self.hist[i].GetListOfFunctions().Add(contact)

    def event(self):
        ''' Event processor: fill histograms '''

        for i in range(self.n):
            hits = Belle2.PyStoreArray(self.simHits[i])
            for hit in hits:
                time = hit.getGlobalTime() / 1000
                self.hist[i].Fill(time)

    def setErrors(self, hist):
        ''' Set bin errors - they are not Poissonian'''
        hmax = hist.GetMaximum()
        temp = TH1F('temp', 'temporary', 100, 0, hmax * 1.1)
        for i in range(hist.GetNbinsX()):
            h = hist.GetBinContent(i + 1)
            if h > hmax * 0.2:
                temp.Fill(h)
        mean = temp.GetMean()
        rms = temp.GetStdDev()
        if mean > 0:
            for i in range(hist.GetNbinsX()):
                h = hist.GetBinContent(i + 1)
                e = 0
                if h > 0:
                    e = rms * math.sqrt(h / mean)
                hist.SetBinError(i + 1, e)

    def terminate(self):
        """ Write histograms to the file."""

        tfile = TFile('timeDistributions.root', 'recreate')
        for i in range(self.n):
            self.setErrors(self.hist[i])
            self.hist[i].Write()
        tfile.Close()


# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# mix beam background
bg = None
if 'BELLE2_BACKGROUND_MIXING_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
    if bg is None:
        B2FATAL('No beam background samples found in folder ' +
                os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    main.add_module(bkgmixer)
else:
    B2FATAL('variable BELLE2_BACKGROUND_MIXING_DIR is not set')

# fill validation histograms
main.add_module(BGHistogrammer())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
