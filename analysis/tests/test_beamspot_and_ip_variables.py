#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


def check_file(input_file_name):
    """Run the check with a quick basf2 printout for the
    first event in ``input_file_name`` on the validation server"""
    input_file = b2tu.require_file(input_file_name, 'validation')
    pa = b2.Path()
    ma.inputMdst("default", input_file, path=pa)
    ma.printVariableValues(
        "", ["IPX", "IPY", "IPZ", "genIPX", "genIPY", "genIPZ"], path=pa)
    b2tu.safe_process(pa, 1)


if __name__ == "__main__":
    b2.set_random_seed(":-)")
    b2tu.configure_logging_for_tests()
    check_file("mdst13.root")
    check_file("mdst14.root")
    # TODO: update this to check a data event when a suitable file is added
