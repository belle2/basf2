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
  <output>timeDistributions.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Runs BG mixer and makes validation histograms</description>
</header>
"""

import basf2 as b2
import os
import glob
import math
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed

b2.set_random_seed(123452)


class BGHistogrammer(b2.Module):

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
        'KLMSimHits',
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
            options = TNamed('MetaOptions', 'shifter')
            self.hist[i].GetListOfFunctions().Add(options)

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
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# mix beam background
bg = None
if 'BELLE2_BACKGROUND_MIXING_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
    if bg is None:
        b2.B2FATAL('No beam background samples found in folder ' +
                   os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    b2.B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    bkgmixer = b2.register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    main.add_module(bkgmixer)
else:
    b2.B2FATAL('variable BELLE2_BACKGROUND_MIXING_DIR is not set')

# fill validation histograms
main.add_module(BGHistogrammer())

# Show progress of processing
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
