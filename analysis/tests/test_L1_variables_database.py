#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma
import b2test_utils as b2tu

#: output trigger names for testing
output_triggers_to_test = ["ffo", "hie", "INVALID_INPUT"]

#: input trigger names for testing
input_triggers_to_test = ["t3_3", "ehigh", "INVALID_INPUT"]

#: trigger bits for testing
bits_to_test = [0, 17, 321]
#                      ^ should be out of range


def check_file(input_file_name, trigger_variables):
    """Run the check with a quick basf2 printout for the
    first event in ``input_file_name`` on the validation server"""
    input_file = b2tu.require_file(input_file_name, '')
    pa = b2.Path()
    ma.inputMdst("default", input_file, path=pa)
    ma.printVariableValues("", trigger_variables, path=pa)
    b2tu.safe_process(pa, 10)


def name_variables(name):
    """List of trigger bits by name access"""
    return f"L1PSNM({name})", f"L1FTDL({name})", f"L1Prescale({name})"


def bit_number_variables(bit):
    """List of trigger bits by bit number access"""
    return f"L1PSNMBit({bit})", f"L1FTDLBit({bit})", f"L1InputBit({bit})", f"L1PSNMBitPrescale({bit})"


if __name__ == "__main__":

    # build the variables
    trigger_variables = [name_variables(name) for name in output_triggers_to_test]
    trigger_variables += [bit_number_variables(bit) for bit in bits_to_test]

    # flatten
    trigger_variables = [var for row in trigger_variables for var in row]
    trigger_variables += [f"L1Input({name})" for name in input_triggers_to_test]
    trigger_variables.sort()

    # test on some files
    b2.set_random_seed(r"\m/")
    b2tu.configure_logging_for_tests()
    check_file("mdst/tests/mdst-v05-00-00.root", trigger_variables)
    # TODO: update this to check a data event when a suitable file is added
