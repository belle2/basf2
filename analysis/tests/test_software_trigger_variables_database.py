#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import b2test_utils as b2tu

#: trigger identifiers for testing in (long form id, short form id) 'tuples
triggers_to_test = [
    ("software_trigger_cut&filter&1_Estargt1_GeV_cluster_no_other_cluster_Estargt0.3_GeV",
     "filter 1_Estargt1_GeV_cluster_no_other_cluster_Estargt0.3_GeV"),
    ("INVALID_INPUT", "this is not a valid trigger identifier")
]


def check_file(input_file_name, trigger_variables):
    """Run the check with a quick basf2 printout for the
    first event in ``input_file_name`` on the validation server"""
    input_file = b2tu.require_file(input_file_name, 'validation')
    pa = b2.Path()
    ma.inputMdst(input_file, path=pa)
    ma.printVariableValues("", trigger_variables, path=pa)
    b2tu.safe_process(pa, 1)


def swtr(name):
    """Make the string form variable name"""
    return "SoftwareTriggerResult(%s)" % name


def swtp(name):
    """Make the string form prescale variable name"""
    return "SoftwareTriggerPrescaling(%s)" % name


if __name__ == "__main__":

    # build the variables
    trigger_variables = [f(lng) for lng, _ in triggers_to_test for f in (swtr, swtp)]
    trigger_variables += [f(shrt) for _, shrt in triggers_to_test for f in (swtr, swtp)]

    # test on some files
    b2.set_random_seed(r"\m/")
    b2tu.configure_logging_for_tests()
    check_file("mdst13.root", trigger_variables)
    # TODO: update this to check a data event when a suitable file is added
