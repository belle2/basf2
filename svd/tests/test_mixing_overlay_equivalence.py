import basf2
import ROOT
from ROOT import Belle2
import b2test_utils

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
# to sort digits differently than ShaperDigitSorter. Both sortings are
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
        super().__init__()
        self.selected_sensorID = Belle2.VxdID(3, 1, 1)

    def event(self):
        global xsimhits
        storesimhits = Belle2.PyStoreArray("SVDSimHits")
        for h in storesimhits:
            if h.getSensorID == self.selected_sensorID:
                xsimhits.append(h)


class InjectSimHits(basf2.Module):
    """Inject stored SVDSimHits"""

    def __init__(self):
        super().__init__()
        self.simhits = Belle2.PyStoreArray("SVDSimHits")

    def initialize(self):
        self.simhits.registerInDataStore()

    def event(self):
        global xsimhits
        for h in xsimhits:
            simhit = self.simhits.appendNew()
            simhit.__assign__(h)


if __name__ == "__main__":

    # Load ROOT libs before we change directory
    dummy_simhit = Belle2.SVDSimHit()
    dummy_digit = Belle2.SVDShaperDigit()
    dummy_bdata = Belle2.BackgroundMetaData()

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
        create_ovrfile.add_module('SVDDigitizer', ElectronicEffects=False)
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
        produce_mixed.add_module('SVDDigitizer', ElectronicEffects=False)
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
        produce_overlaid.add_module('SVDDigitizer', ElectronicEffects=False)
        produce_overlaid.add_module('BGOverlayExecutor')
        # Sort digits after overlay
        produce_overlaid.add_module('SVDShaperDigitSorter')
        produce_overlaid.add_module('RootOutput', outputFileName='overlaidBg.root')
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(produce_overlaid)

        basf2.B2INFO('Comparing...')

        # Compare diigts stored in the two root files
        f_mixed = ROOT.TFile('mixedBg.root')
        tree_mixed = f_mixed.Get('tree')

        f_over = ROOT.TFile('overlaidBg.root')
        tree_over = f_over.Get('tree')

        # We only have a single event in both files
        n_good = 0
        for evt1, evt2 in zip(tree_mixed, tree_over):
            d_mixed = evt1.SVDShaperDigits
            d_over = evt2.SVDShaperDigits
            if (len(d_mixed) != len(d_over)):
                basf2.B2ERROR('Lengths of SVDShaperDigit arrays DIFFER!')
                break
            for d1, d2 in zip(d_mixed, d_over):
                match = True
                match = match and (d1.getSensorID() == d2.getSensorID())
                match = match and (d1.isUStrip() == d2.isUStrip())
                match = match and (d1.getCellID() == d2.getCellID())
                for s1, s2 in zip(d1.getSamples(), d2.getSamples()):
                    match = match and (int(s1) == int(s2))
                match = match and (d1.getModeByte() == d2.getModeByte())
                if not match:
                    print(d1.toString())
                    print(d2.toString())
                    basf2.B2ERROR('Digits do not match.')
                else:
                    n_good += 1

        basf2.B2INFO('Processed {0} matching digits.'.format(n_good))
