#!/usr/bin/env python
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
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed

set_random_seed(123452)


class BGHistogrammer(Module):

    '''
    Make validation histograms for BG mixer.
    '''

    ## list of SimHits to be histogramed
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
    ## length of simHits list
    n = len(simHits)
    ## list of histograms
    hist = [TH1F('h' + str(i), simHits[i], 400, -20, 20) for i in range(n)]

    def initialize(self):
        ''' Initialize the Module: set histogram axis titles, description and check'''

        for i in range(self.n):
            self.hist[i].GetXaxis().SetTitle('time [#mus]')
            self.hist[i].GetYaxis().SetTitle('entries/bin')
            descr = TNamed('Description', 'Time distribution of '
                           + self.simHits[i] + ' for mixed background')
            self.hist[i].GetListOfFunctions().Add(descr)
            check = TNamed('Check',
                           'Distribution must be flat in the time window. '
                           + 'Bin statistics is not Poissonian.')
            self.hist[i].GetListOfFunctions().Add(check)

    def event(self):
        ''' Event processor: fill histograms '''

        for i in range(self.n):
            hits = Belle2.PyStoreArray(self.simHits[i])
            for hit in hits:
                time = hit.getGlobalTime() / 1000
                self.hist[i].Fill(time)

    def terminate(self):
        """ Write histograms to the file."""

        tfile = TFile('timeDistributions.root', 'recreate')
        for i in range(self.n):
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
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    main.add_module(bkgmixer)
else:
    print 'Warning: variable BELLE2_BACKGROUND_DIR is not set'

# fill validation histograms
main.add_module(BGHistogrammer())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics

# Print warning again
if bg is None:
    print 'Warning: variable BELLE2_BACKGROUND_DIR is not set -> BG mixer not in path!'
    print
