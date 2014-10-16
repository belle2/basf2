#!/usr/bin/env python
# -*- coding: utf-8 -*-

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import glob
import sys

from ROOT import Belle2
from ROOT import TH1F, TH2F, TCanvas
from simulation import add_simulation
from reconstruction import add_reconstruction


# ------------------------------------------------------------------------------
# An example of a simple event display, made by using a python module.
# It displays timing distributions of modules produced by a generic BBbar event.
# Red histograms are marking the modules that are hited by reconstructed tracks.
# ------------------------------------------------------------------------------

class TOPDisplay(Module):

    hist = [TH1F('h' + str(i), 'bar#' + str(i), 128, 0.0, 4096.0) for i in
            range(16)]
    c1 = TCanvas('c1', 'TOP event display', 1000, 800)

    def initialize(self):

        self.c1.Divide(4, 4)
        self.c1.Show()

    def event(self):

        for i in range(16):
            self.hist[i].Reset()
            self.hist[i].SetFillColor(0)
            self.hist[i].SetLineColor(1)

        likelihoods = Belle2.PyStoreArray('TOPLikelihoods')
        for likelihood in likelihoods:
            if likelihood.getFlag() == 1:
                exthit = likelihood.getRelated('ExtHits')
                try:
                    barID = exthit.getCopyID()
                    self.hist[barID - 1].SetFillColor(2)
                    self.hist[barID - 1].SetLineColor(2)
                except:
                    B2ERROR('No relation to ExtHit')

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            barID = digit.getBarID()
            tdc = digit.getTDC()
            self.hist[barID - 1].Fill(tdc)

        for i in range(16):
            self.c1.cd(i + 1)
            self.hist[i].Draw()

        self.c1.Update()

        try:
            q = 0
            Q = 0
            abc = input('Type <CR> to continue or Q to quit ')
            evtMetaData = Belle2.PyStoreObj('EventMetaData')
            evtMetaData.obj().setEndOfData()
        except:
            abc = ''  # dummy line to terminate try-except


# Check if the display is set (needed for canvas)
if not os.environ.has_key('DISPLAY'):
    print 'DISPLAY variable is not set'
    print '- to set it in bash: export DISPLAY=:0'
    print '- to set it in csh:  setenv DISPLAY :0'
    sys.exit()

# Suppress messages other than errors
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# Detector simulation
bg = None
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# Reconstruction
add_reconstruction(main)

# dispay
main.add_module(TOPDisplay())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics
