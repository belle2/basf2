#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond SUPPRESS_DOXYGEN

"""
<header>
<output>validationTestEmptyRootFile.root</output>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>


<description>
This file will not generate an output root file to probe
the validation suite properly handles this case.
</description>
</header>
"""

from ROOT import TFile


VALIDATION_OUTPUT_FILE = "validationTestEmptyRootFile.root"


if __name__ == "__main__":
    tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
    tfile.Close()

# @endcond
