##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from ROOT import Belle2
import b2test_utils

# ====================================================================
# This test does the following:
# 1. Simulate a few hundred ParticleGun events to produce a sample of
#    SVD simhits.
# 2. Use SVDDigitizer with varius zero-suppression cut settings and check
#    that the settings work.
# ====================================================================


class CheckZS(basf2.Module):
    """Check that SVVDShaperDigits are correctly zero-suppressed."""

    def __init__(self, thresholdSN):
        '''initialize python module'''
        super().__init__()

        #: SNR threshold
        self.thresholdSN = thresholdSN

    def event(self):
        '''event'''
        storedigits = Belle2.PyStoreArray("SVDShaperDigits")
        gc = Belle2.VXD.GeoCache.getInstance()
        for digit in storedigits:
            info = gc.get(digit.getSensorID())
            isU = digit.isUStrip()
            noise_in_e = info.getElectronicNoiseU() if isU else info.getElectronicNoiseV()
            adu_equivalent = info.getAduEquivalentU() if isU else info.getAduEquivalentV()
            noise_in_adu = noise_in_e / adu_equivalent
            threshold_in_adu = self.thresholdSN * noise_in_adu
            samples = digit.getSamples()
            for s in samples:
                if s > threshold_in_adu:
                    break
            else:
                basf2.B2FATAL("Found digit under threshold: {0}".format(digit.to_string()))


if __name__ == "__main__":

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('Creating simhits...')
        # We create data separately, so that we can use them in other tests.

        create_simhits = basf2.create_path()
        create_simhits.add_module('EventInfoSetter', evtNumList=[50])
        create_simhits.add_module('Gearbox')
        create_simhits.add_module('Geometry', components=['MagneticField', 'SVD'], useDB=False)
        create_simhits.add_module('ParticleGun')
        create_simhits.add_module('FullSim')
        create_simhits.add_module('RootOutput', outputFileName='SimulationForThresholdTest.root',
                                  branchNames=['EventMetaData', 'SVDSimHits', 'SVDTrueHits'])
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(create_simhits)

        basf2.B2INFO('Threshold testing...')

        threshold = 3

        test_threshold = basf2.create_path()
        # Read generated Simhits and Truehits
        test_threshold.add_module('RootInput', inputFileName='SimulationForThresholdTest.root')
        test_threshold.add_module('Gearbox')
        test_threshold.add_module('Geometry', components=['MagneticField', 'SVD'], useDB=False)
        # Set the threshold.
        test_threshold.add_module('SVDDigitizer', ZeroSuppressionCut=threshold)
        threshold_checker = CheckZS(threshold)
        test_threshold.add_module(threshold_checker)
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(test_threshold)

        basf2.B2INFO('Test done.')
