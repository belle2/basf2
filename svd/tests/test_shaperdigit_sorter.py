import os
import tempfile
import basf2
import ROOT
from ROOT import Belle2
import multiprocessing
from contextlib import contextmanager
from svd import add_svd_reconstruction


@contextmanager
def clean_working_directory():
    """Context manager to create a temporary directory and directly us it as
    current working directory"""
    dirname = os.getcwd()
    try:
        with tempfile.TemporaryDirectory() as tempdir:
            os.chdir(tempdir)
            yield tempdir
    finally:
        os.chdir(dirname)


def sub_process(path):
    """Run process() in a subprocess to avoid side effects"""
    p = multiprocessing.Process(target=basf2.process, args=(path,))
    p.start()
    p.join()


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
        for d in digits:
            print("digit: sensor %s, u-side %s, strip %d" %
                  (d.getSensorID(), d.isUStrip(), d.getCellID()))
        for c in clusters:
            print("cluster: sensor %s, position %d  size %d" %
                  (c.getSensorID(), c.getPosition(), c.getSize()))


if __name__ == "__main__":
    # Load the dictionary before changing directory, otherwise ROOT won't find it.
    dummy = Belle2.SVDShaperDigit()
    with clean_working_directory():
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

        test_ordering = basf2.create_path()
        test_ordering.add_module("EventInfoSetter")
        test_ordering.add_module("Gearbox")
        test_ordering.add_module("Geometry", components=["SVD"])
        test_ordering.add_module(CreateDigits(sig_digits))
        test_ordering.add_module("SVDShaperDigitSorter")
        add_svd_reconstruction(test_ordering, useCoG=True)
        test_ordering.add_module(PrintDigitsAndClusters())
        test_ordering.add_module(CheckOrderingOfDigits())
        sub_process(test_ordering)
