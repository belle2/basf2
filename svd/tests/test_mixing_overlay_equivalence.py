##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import ROOT
from ROOT import Belle2
import b2test_utils
from svd import add_svd_simulation

# ====================================================================
# This test does the following:
# 1. Simulate a few hundred ParticleGun events to produce a background
#    sample.
# 2. Produce background overlay sample from background sample. .
# 3. Set aside SimHits from a selected sensor to create signal-background
#    collisions.
# 4. Inject collision SimHits, add background using the background mixer,
#    and digitize. Sort and save.
# 5. Inject collision SimHits, digitize, and use background overlay
#    to add background. Sort and save.
# 7. Compare SVDShaperDigits in the two ROOT files.
#
# NB: I sort both digit samples before saving. Normally, the digits
# require sorting only after background overlay, but SVDDigitizer happens
# to sort digits differently than ShaperDigitSorter. Both sorting are
# correct, but the sorting to eventually correct is that in the
# digitizer.
# NB: Switching off electronic effects in SVDDigitizer prevents it from
# adding noise digits.
# ====================================================================


xsimhits = []


class SetAsideSimHits(basf2.Module):
    """Set aside all SimHits on a selected sensor/side to create
        signal/background collisions."""

    def __init__(self):
        '''initialize python module'''
        super().__init__()

        #: selected sensor info
        self.selected_sensorID = Belle2.VxdID(3, 1, 1)

    def event(self):
        '''event'''
        global xsimhits
        storesimhits = Belle2.PyStoreArray("SVDSimHits")
        for h in storesimhits:
            if h.getSensorID == self.selected_sensorID:
                xsimhits.append(h)


class InjectSimHits(basf2.Module):
    '''Inject stored SVDSimHits'''

    def __init__(self):
        '''initialize python module'''
        super().__init__()
        #: simHits store array

        self.simhits = Belle2.PyStoreArray("SVDSimHits")

    def initialize(self):
        '''initialize'''
        self.simhits.registerInDataStore()

    def event(self):
        '''event'''

        global xsimhits
        for h in xsimhits:
            simhit = self.simhits.appendNew()
            simhit.__assign__(h)


if __name__ == "__main__":

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('Creating background data...')

        create_bgfile = basf2.create_path()
        create_bgfile.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[100])
        create_bgfile.add_module('Gearbox')
        create_bgfile.add_module('Geometry', components=['MagneticField', 'SVD'])
        create_bgfile.add_module('ParticleGun')
        create_bgfile.add_module('FullSim')
        create_bgfile.add_module('BeamBkgTagSetter', backgroundType="twoPhoton", realTime=310.0)
        create_bgfile.add_module('RootOutput', outputFileName='bgForMixing.root', branchNames=['BackgroundMetaData', 'SVDSimHits'])
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(create_bgfile)

        basf2.B2INFO('Creating overlay data...')

        create_ovrfile = basf2.create_path()
        create_ovrfile.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        create_ovrfile.add_module('Gearbox')
        create_ovrfile.add_module('Geometry', components=['MagneticField', 'SVD'])
        create_ovrfile.add_module('BeamBkgMixer', backgroundFiles=['bgForMixing.root'], minTime=-150, maxTime=150)
        # Turn off generation of noise digits in SVDDigitizer.
        add_svd_simulation(create_ovrfile)

        create_ovrfile.add_module(SetAsideSimHits())
        create_ovrfile.add_module('RootOutput', outputFileName='bgForOverlay.root', branchNames=['SVDShaperDigits'])
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(create_ovrfile)

        basf2.B2INFO('Background mixing...')

        produce_mixed = basf2.create_path()
        produce_mixed.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        produce_mixed.add_module('Gearbox')
        produce_mixed.add_module('Geometry', components=['MagneticField', 'SVD'])
        # Inject stored simhits
        produce_mixed.add_module(InjectSimHits())
        # Time window for background file to just cover one event
        produce_mixed.add_module('BeamBkgMixer', backgroundFiles=['bgForMixing.root'], minTime=-150, maxTime=150)
        # Turn off generation of noise digits in SVDDigitizer.
        add_svd_simulation(produce_mixed)

        produce_mixed.add_module('SVDShaperDigitSorter')
        produce_mixed.add_module('RootOutput', outputFileName='mixedBg.root')
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(produce_mixed)

        basf2.B2INFO('Background overlay...')

        produce_overlaid = basf2.create_path()
        produce_overlaid.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        produce_overlaid.add_module('Gearbox')
        produce_overlaid.add_module('Geometry', components=['MagneticField', 'SVD'])
        produce_overlaid.add_module('BGOverlayInput', inputFileNames=['bgForOverlay.root'])
        produce_overlaid.add_module(InjectSimHits())

        add_svd_simulation(produce_overlaid)

        produce_overlaid.add_module('BGOverlayExecutor')
        # Sort digits after overlay
        produce_overlaid.add_module('SVDShaperDigitSorter')
        produce_overlaid.add_module('RootOutput', outputFileName='overlaidBg.root')
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(produce_overlaid)

        basf2.B2INFO('Comparing...')

        with ROOT.TFile('mixedBg.root') as f_mixed:
            tree_mixed = f_mixed.Get('tree')

            with ROOT.TFile('overlaidBg.root') as f_over:
                tree_over = f_over.Get('tree')

                class_name = "Belle2::SVDShaperDigit"

                digits_mixed = ROOT.TClonesArray(class_name)
                digits_over = ROOT.TClonesArray(class_name)

                tree_mixed.SetBranchAddress("SVDShaperDigits", digits_mixed)
                tree_over.SetBranchAddress("SVDShaperDigits", digits_over)

                n_good = 0

                for i in range(tree_mixed.GetEntries()):
                    tree_mixed.GetEntry(i)
                    tree_over.GetEntry(i)

                    n_mixed = digits_mixed.GetEntriesFast()
                    n_over = digits_over.GetEntriesFast()

                    if n_mixed != n_over:
                        basf2.B2FATAL('Lengths of SVDShaperDigit arrays differ')
                        break

                    for d1, d2 in zip(digits_mixed, digits_over):
                        match = (
                            d1.getSensorID() == d2.getSensorID() and
                            d1.isUStrip() == d2.isUStrip() and
                            d1.getCellID() == d2.getCellID()
                        )

                        samples1 = d1.getSamples()
                        samples2 = d2.getSamples()

                        if len(samples1) != len(samples2):
                            match = False
                        else:
                            for s1, s2 in zip(samples1, samples2):
                                if int(s1) != int(s2):
                                    match = False
                                    break

                        if not match:
                            print(d1.toString())
                            print(d2.toString())
                            basf2.B2FATAL('Digits do not match.')
                        else:
                            n_good += 1

                basf2.B2INFO(f'Processed {n_good} matching digits.')
