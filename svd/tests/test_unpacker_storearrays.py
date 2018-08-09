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
test_successful = True
test_message = ['successful', 'FAILED']
# ---------------------------------------------------------------------

if __name__ == "__main__":

    # Load ROOT libs before we change directory
    dummy_raw = Belle2.RawSVD()
    dummy_digit = Belle2.SVDDigit()
    dummy_shaper = Belle2.SVDShaperDigit()

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('Test 1. \nBehaviour with residual digits\n')

        basf2.B2INFO('1/6 \nSet up: Using ParticleGun and SVDPacker, we generate\n' +
                     'a file with RawSVDs and residual digits.')

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
            result = b2test_utils.safe_process(create_input)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Generation {0}.\n'.format(test_message[result]))

        basf2.B2INFO('2/6\nRead using default settings of SVDUnpacker.' +
                     '\nWe should see FATAL with message for SVDDigits.')

        read_digits_default = basf2.create_path()
        read_digits_default.add_module('RootInput', inputFileName='rawPlusDigits.root')
        read_digits_default.add_module('Gearbox')
        read_digits_default.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_digits_default.add_module('SVDUnpacker', GenerateOldDigits=True)
        add_svd_reconstruction(read_digits_default)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(read_digits_default)

        test_successful = test_successful and (result == 1)
        basf2.B2INFO('Test {0}.\n'.format(test_message[1 - result]))

        basf2.B2INFO('3/6\nRead shaper digits using default settings of SVDUnpacker.' +
                     '\nWe should see FATAL with message for SVDShaperDigits.')

        read_shapers_default = basf2.create_path()
        read_shapers_default.add_module('RootInput', inputFileName='rawPlusDigits.root', excludeBranchNames=['SVDDigits'])
        read_shapers_default.add_module('Gearbox')
        read_shapers_default.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_shapers_default.add_module('SVDUnpacker')
        add_svd_reconstruction(read_shapers_default)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(read_shapers_default)

        test_successful = test_successful and (result == 1)
        basf2.B2INFO('Test {0}.\n'.format(test_message[1 - result]))

        basf2.B2INFO('4/6\nRead the safe way, excluding branches.\nWe should see no errors.')

        read_safe_way = basf2.create_path()
        read_safe_way.add_module('RootInput',
                                 inputFileName='rawPlusDigits.root',
                                 branchNames=['EventMetaData', 'RawSVDs'])
        read_safe_way.add_module('Gearbox')
        read_safe_way.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_safe_way.add_module('SVDUnpacker')
        add_svd_reconstruction(read_safe_way)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(read_safe_way)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Test {0}.\n'.format(test_message[result]))

        basf2.B2INFO('5/6\nRead with the Unpacker silentlyAppend switch on.\n' +
                     'We also include the SVDShaperDigitSorter.\n' +
                     'We should see no errors.')

        read_append_sort = basf2.create_path()
        read_append_sort.add_module('RootInput',
                                    inputFileName='rawPlusDigits.root',
                                    branchNames=['EventMetaData', 'RawSVDs', 'SVDShaperDigits'])
        read_append_sort.add_module('Gearbox')
        read_append_sort.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_append_sort.add_module('SVDUnpacker', silentlyAppend=True)
        read_append_sort.add_module('SVDShaperDigitSorter')
        add_svd_reconstruction(read_append_sort)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(read_append_sort)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Test {0}.\n'.format(test_message[result]))

        basf2.B2INFO('6/6\nRead with the Unpacker silentAppend switch on.\n' +
                     'We don\'t include the SVDShaperDigitSorter.\n' +
                     'THERE WILL BE NO ERRORS, but digits and clusters may be duplicated!')

        read_append_nosort = basf2.create_path()
        read_append_nosort.add_module('RootInput',
                                      inputFileName='rawPlusDigits.root',
                                      branchNames=['EventMetaData', 'RawSVDs', 'SVDShaperDigits'])
        read_append_nosort.add_module('Gearbox')
        read_append_nosort.add_module('Geometry', components=['MagneticField', 'SVD'])
        read_append_nosort.add_module('SVDUnpacker', silentlyAppend=True)
        add_svd_reconstruction(read_append_nosort)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(read_append_nosort)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Test {0}.\n'.format(test_message[result]))

        basf2.B2INFO('6 tasks of Test 1 completed\n\n')

        basf2.B2INFO('Test 2\nBehaviour when data absent\n')

        basf2.B2INFO('1/2\nRun unpacking and reconstruction with data.' +
                     '\nAll settings are standard.' +
                     '\nWe should see no errors. ')

        unpack_with_data = basf2.create_path()
        unpack_with_data.add_module(
            'RootInput',
            inputFileName='rawPlusDigits.root',
            excludeBranchNames=[
                'SVDDigits',
                'SVDShaperDigits'])
        unpack_with_data.add_module('Gearbox')
        unpack_with_data.add_module('Geometry', components=['MagneticField', 'SVD'])
        add_svd_unpacker(unpack_with_data)
        add_svd_reconstruction(unpack_with_data)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(unpack_with_data)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Test {0}.\n'.format(test_message[result]))

        basf2.B2INFO('2/2\nRun unpacking and reconstruction without data.' +
                     '\nAll settings are standard.' +
                     '\nWe should see no errors. ')

        unpack_without_data = basf2.create_path()
        unpack_without_data.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        unpack_without_data.add_module('Gearbox')
        unpack_without_data.add_module('Geometry', components=['MagneticField', 'SVD'])
        add_svd_unpacker(unpack_without_data)
        add_svd_reconstruction(unpack_without_data)

        with b2test_utils.show_only_errors():
            result = b2test_utils.safe_process(unpack_without_data)

        test_successful = test_successful and (result == 0)
        basf2.B2INFO('Test {0}.\n'.format(test_message[result]))

        basf2.B2INFO('2 tasks of Test 2 completed\n')

        basf2.B2INFO('Summary result:\nTests {0}.\n'.format(
            test_message[0 if test_successful else 1]))
