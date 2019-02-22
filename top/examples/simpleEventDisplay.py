#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import os
import glob
import sys

from ROOT import Belle2
from ROOT import TH1F, TH2F, TCanvas
from simulation import add_simulation
from reconstruction import add_reconstruction


# ------------------------------------------------------------------------------
# An example of a simple event display, made by using a python module.
# It displays timing distributions of modules produced by a generic BBbar event.
# Red histograms are marking the modules with at least one track impact.
# ------------------------------------------------------------------------------

class TOPDisplay(Module):

    '''
    Simple event display for TOP.
    It displays single event timing distributions of modules using TOPDigits.
    Distributions of modules with the track impact are shown in red color.
    '''

    #: 1D histograms
    hist = [TH1F('h' + str(i), 'module#' + str(i), 128, 0.0, 256.0) for i in
            range(16)]
    #: canvas
    c1 = TCanvas('c1', 'TOP event display', 1000, 800)

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''

        self.c1.Divide(4, 4)
        self.c1.Show()

    def event(self):
        '''
        Event processor: reset and fill the histograms, display them,
        wait for user respond.
        '''

        for i in range(16):
            self.hist[i].Reset()
            self.hist[i].SetFillColor(0)
            self.hist[i].SetLineColor(1)

        likelihoods = Belle2.PyStoreArray('TOPLikelihoods')
        for likelihood in likelihoods:
            if likelihood.getFlag() == 1:
                exthit = likelihood.getRelated('ExtHits')
                try:
                    moduleID = exthit.getCopyID()
                    self.hist[moduleID - 1].SetFillColor(2)
                    self.hist[moduleID - 1].SetLineColor(2)
                except:
                    B2ERROR('No relation to ExtHit')

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            moduleID = digit.getModuleID()
            tdc = digit.getRawTime()
            self.hist[moduleID - 1].Fill(tdc)

        for i in range(16):
            self.c1.cd(i + 1)
            self.hist[i].Draw()

        self.c1.Update()

        # wait for user respond
        try:
            q = 0
            Q = 0
            abc = eval(input('Type <CR> to continue or Q to quit '))
            evtMetaData = Belle2.PyStoreObj('EventMetaData')
            evtMetaData.obj().setEndOfData()
        except:
            abc = ''  # dummy line to terminate try-except


# Check if the display is set (needed for canvas)
if 'DISPLAY' not in os.environ:
    print('DISPLAY variable is not set')
    print('- to set it in bash: export DISPLAY=:0')
    print('- to set it in csh:  setenv DISPLAY :0')
    sys.exit()

# Suppress messages other than errors
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# Detector simulation
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
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
print(statistics)
