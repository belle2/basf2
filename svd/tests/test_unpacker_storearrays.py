import basf2
from svd import *
import ROOT
from ROOT import Belle2
import b2test_utils

# ====================================================================
# Here we test the following behaviour of SVDUnpacker:
# 1. In case there is a pre-existing non-empty StoreArray of SVDDigits
#    or SVDShaperDigits, the unpacker throws B2FATAL unless the
#    silentAppend parameter of the module is set.
#    RATIONALE: This situation leads to unsorted digit arrays, and
#    it can occur by error easily (by forgetting to exclude digits from
#    HLT processing, for example). Unless a digit sorter is used, it
#    will break clustering. Therefore, turning of a module parameter is
#    required to make sure the user knows what he/she is doing.
# 2. In case there are no RawSVDs on input, the unpacker silently does
#    nothing, only registers the output arrays.
#    RATIONALE: This is meant to simplify processing, so that it be
#    not necessary to customize processing scripts for each detector
#    configuration. SVD reconstruction chain should silently (or with
#    a single warning) withstand an absence of SVD data.
# ====================================================================


class insert_svd_digits(basf2.Module):
    """
    Register SVDDigits StoreArray in the DataStore and add some digits.
    """

    def __init__(self, ndigits=0):
        super().__init__()
        self.ndigits = ndigits

    def initialize(self):
        """ Register SVDDigits StoreArray """
        svd_digits = Belle2.PyStoreArray(Belle2.SVDDigit.Class())
        svd_digits.registerInDataStore()

    def event(self):
        store_digits = Belle2.PyStoreArray("SVDDigits")
        for i in range(self.ndigits):
            # Make 6-sample groups, but don't care too much.
            digit = Belle2.SVDDigit(Belle2.VxdID(3, 1, 1), True,
                                    i / 6, (i / 6) * 2.0, 30 + i % 6, i % 6)
            place = store_digits.appendNew()
            place.__assign__(digit)


class insert_shaper_digits(basf2.Module):
    """
    Register SVDShaperDigits StoreArray in the DataStore and add
    some digits.
    """

    def __init__(self, ndigits=0):
        super().__init__()
        self.ndigits = ndigits

    def initialize(self):
        """ Register SVDDigits StoreArray """
        svd_digits = Belle2.PyStoreArray(Belle2.SVDShaperDigit.Class())
        svd_digits.registerInDataStore()

    def event(self):
        store_digits = Belle2.PyStoreArray("SVDShaperDigits")
        for i in range(self.ndigits):
            digit = Belle2.SVDShaperDigit(Belle2.VxdID(3, 1, 1), True, 30 + i,
                                          [0, 0, 30, 35, 25, 15])
            place = store_digits.appendNew()
            place.__assign__(digit)


class insert_raw_svds(basf2.Module):
    """
    Register RawSVDs StoreArray in the DataStore and add
    some data.
    """

    def __init__(self, nobjects=0):
        super().__init__()
        self.nobjects = nobjects

    def initialize(self):
        """ Register SVDDigits StoreArray """
        svd_raws = Belle2.PyStoreArray(Belle2.RawSVD.Class())
        svd_raw.registerInDataStore()

    def event(self):
        store_raws = Belle2.PyStoreArray("RawSVDs")
        for i in range(self.nobjects):
            araw = Belle2.RawSVD()
            place = store_raws.appendNew()
            place.__assign__(araw)


if __name__ == "__main__":

    # Load ROOT libs before we change directory
    dummy_raw = Belle2.RawSVD()
    dummy_digit = Belle2.SVDDigit()
    dummy_shaper = Belle2.SVDShaperDigit()

    basf2.B2INFO('\nTest 1. Behaviour with residual digits\n')

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('\n1.1. Set up: Generate file with residual digits and RawSVDs.\n')

        create_input = basf2.create_path()
        create_input.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        create_input.add_module('Gearbox')
        create_input.add_module('Geometry', components=['MagneticField', 'SVD'])
        create_input.add_module('ParticleGun')
        create_input.add_module('FullSim')

        create_input.add_module('SVDDigitizer', GenerateDigits=True)
        create_input.add_module('SVDPacker')
        create_input.add_module('RootOutput', outputFileName='rawPlusDigits.root')
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(create_input)

        basf2.B2INFO('Generation completed. No errors should have appeared.')

        basf2.B2INFO('1.2. Read using default settings of SVDUnpacker.' +
                     '\nWe should see FATAL with message for SVDDigits.')

        read_digits_default = basf2.create_path()
        read_digits_default.add_module('RootInput', inputFileName='rawPlusDigits.root')
        read_digits_default.add_module('Gearbox')
        read_digits_default.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_digits_default.add_module('SVDUnpacker', GenerateShaperDigits=False)
        add_svd_reconstruction(read_digits_default)

        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(read_digits_default)

        basf2.B2INFO('Test completed.\n')

        basf2.B2INFO('1.3. Read shaper digits using default settings of SVDUnpacker.' +
                     '\nWe should see FATAL with message for SVDShaperDigits.')

        read_shapers_default = basf2.create_path()
        read_shapers_default.add_module('RootInput', inputFileName='rawPlusDigits.root', excludeBranchNames=['SVDDigits'])
        read_shapers_default.add_module('Gearbox')
        read_shapers_default.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_shapers_default.add_module('SVDUnpacker', GenerateShaperDigits=True)
        add_svd_reconstruction(read_shapers_default)

        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(read_shapers_default)

        basf2.B2INFO('Test completed.\n')

        basf2.B2INFO('1.4. Read the safe way. We should see no errors.')

        read_safe_way = basf2.create_path()
        read_safe_way.add_module('RootInput',
                                 inputFileName='rawPlusDigits.root',
                                 branchNames=['EventMetaData', 'RawSVDs'])
        read_safe_way.add_module('Gearbox')
        read_safe_way.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_safe_way.add_module('SVDUnpacker', GenerateShaperDigits=True)
        add_svd_reconstruction(read_safe_way)

        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(read_safe_way)

        basf2.B2INFO('Test completed.\n')

        basf2.B2INFO('1.5. Read with the Unpacker silentlyAppend switch.\n' +
                     'We also include the SVDShaperDigitSorter.\n' +
                     'We should see no errors.')

        read_append_sort = basf2.create_path()
        read_append_sort.add_module('RootInput',
                                    inputFileName='rawPlusDigits.root',
                                    branchNames=['EventMetaData', 'RawSVDs', 'SVDShaperDigits'])
        read_append_sort.add_module('Gearbox')
        read_append_sort.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_append_sort.add_module('SVDUnpacker', GenerateShaperDigits=True, silentlyAppend=True)
        read_append_sort.add_module('SVDShaperDigitSorter')
        add_svd_reconstruction(read_append_sort)

        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(read_append_sort)

        basf2.B2INFO('Test completed.\n')

        basf2.B2INFO('1.6. Read with the Unpacker silentAppend switch.\n' +
                     'We don\'t include the SVDShaperDigitSorter.\n' +
                     'BUT THERE WILL BE NO ERRORS!')

        read_append_nosort = basf2.create_path()
        read_append_nosort.add_module('RootInput',
                                      inputFileName='rawPlusDigits.root',
                                      branchNames=['EventMetaData', 'RawSVDs', 'SVDShaperDigits'])
        read_append_nosort.add_module('Gearbox')
        read_append_nosort.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_append_nosort.add_module('SVDUnpacker', GenerateShaperDigits=True, silentlyAppend=True)
        add_svd_reconstruction(read_append_nosort)

        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(read_append_nosort)

        basf2.B2INFO('Test completed.\n')
        basf2.B2INFO('Test 1 completed: pre-existing digits\n\n')
