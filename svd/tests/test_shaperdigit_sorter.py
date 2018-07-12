import os
import tempfile
import basf2
import ROOT
from ROOT import Belle2
import b2test_utils
from svd import add_svd_reconstruction


class CreateDigits(basf2.Module):
    """Create SVD ShaperDigits"""

    def __init__(self, digits):
        """digits is a list of (sensor, side, strip) for which to generate a digit"""
        super().__init__()
        self.svddigits = Belle2.PyStoreArray("SVDShaperDigits")
        self.digits = digits
        self.samples = ROOT.array('unsigned char', 6)()
        for i, sample in zip(range(6), [0, 0, 12, 18, 12, 8]):
            self.samples[i] = sample
        self.mode = Belle2.SVDModeByte(144)
        self.fadc_time = 0

    def initialize(self):
        self.svddigits.registerInDataStore()

    def event(self):
        for sensor, side, strip in self.digits:
            d = self.svddigits.appendNew()
            d.__assign__(Belle2.SVDShaperDigit(Belle2.VxdID(3, 1, sensor), side, strip, self.samples, self.fadc_time, self.mode))
        print('\nGenerated digits:')
        for digit in self.svddigits:
            print("digit: sensor %s, side %s, strip %d" %
                  (digit.getSensorID(), 'u' if digit.isUStrip() else 'v', digit.getCellID()))


class CheckOrderingOfDigits(basf2.Module):
    """Check ordering of SVD digits"""

    def event(self):
        digits = Belle2.PyStoreArray("SVDShaperDigits")
        current_ID = 0
        for d in digits:
            new_ID = d.getUniqueChannelID()
            if new_ID > current_ID:
                current_ID = new_ID
            else:
                Belle2.B2ERROR('Wrong ordering!')


class PrintDigitsAndClusters(basf2.Module):
    """Print all SVD digits and clusters"""

    def event(self):
        digits = Belle2.PyStoreArray("SVDShaperDigits")
        clusters = Belle2.PyStoreArray("SVDClusters")
        print('\nSorted digits and clusters:')
        for d in digits:
            print("digit: sensor %s, u-side %s, strip %d" %
                  (d.getSensorID(), d.isUStrip(), d.getCellID()))
        for c in clusters:
            print("cluster: sensor %s, side %s, position %d  size %d" %
                  (c.getSensorID(), 'u' if c.isUCluster() else 'v', c.getPosition(), c.getSize()))


if __name__ == "__main__":
    # Load the dictionary before changing directory, otherwise ROOT won't find it.
    dummy = Belle2.SVDShaperDigit()
    with b2test_utils.clean_working_directory():
        # basf2.set_log_level(basf2.LogLevel.ERROR)
        sig_digits = [
            (2, False, 112),
            (2, True, 66),
            (1, True, 12),
            (2, True, 65),
            (1, True, 11),
            (2, False, 113),
            (1, True, 10)
        ]

        basf2.B2INFO('Test SVDShaperDigitSorter\n' +
                     'In this test, we generate some ShaperDigits in  radnom order\n' +
                     'and insert them in the DataStore. Then we call shaper digit\n' +
                     'sorter module and check whether the digits are correctly sorted.\n')

        test_ordering = basf2.create_path()
        test_ordering.add_module("EventInfoSetter")
        test_ordering.add_module("Gearbox")
        test_ordering.add_module("Geometry", components=["SVD"])
        test_ordering.add_module(CreateDigits(sig_digits))
        test_ordering.add_module("SVDShaperDigitSorter")
        add_svd_reconstruction(test_ordering, useCoG=True)
        test_ordering.add_module(PrintDigitsAndClusters())
        test_ordering.add_module(CheckOrderingOfDigits())

        result = b2test_utils.safe_process(test_ordering)
        basf2.B2INFO('\nTest finished {0}.\n'.format(
            ['successfully', 'with failure'][result]))
