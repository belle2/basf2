#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Create sphinx documentation and make sure that there are no warnings.
However, a few warnings can not easily be omitted and are therefore accepted for now.
"""

import re
from b2test_utils import check_error_free

#: light build
light_build = False
try:
    import generators  # noqa
except ModuleNotFoundError:
    light_build = True

if __name__ == "__main__":
    #: ignore further warnings in light builds
    #: ignore online_book
    ignoreonlinebook = 'online_book'
    #: ignore add_trigger_simulation
    ignoreaddtriggersimulation = 'add_trigger_simulation'
    #: ignore add_simulation of simulation package
    ignoreaddsimulation = 'add_simulation'
    #: ignore add_reconstruction of reconstruction package
    ignoreaddreconstruction = 'add_reconstruction'
    #: ignore add_cdst_output of reconstruction package
    ignoreaddcdstoutput = 'add_cdst_output'
    #: ignore validation_tools of validation package
    ignorevalidationtools = 'validation_tools'
    #: ignore missing include directives
    ignoreincludeproblem = 'Problems with "include" directive path'
    #: ignore missing track matching link
    ignoretrackmatching = 'trk_matching'
    #: ignore missing tracking_eventtimeextraction
    ignoretrackingeventtimeextraction = 'tracking_eventtimeextraction'
    #: ignore wrong link to tools doc until tools are updated
    ignoretoolsdoc = 'cvmfs_setup'

    check_error_free("b2code-sphinx-warnings", "sphinx", None,
                     lambda x:
                     re.findall(ignoreaddsimulation, x) or
                     re.findall(ignoreaddtriggersimulation, x) or
                     re.findall(ignoreaddreconstruction, x) or
                     re.findall(ignoreaddcdstoutput, x) or
                     re.findall(ignorevalidationtools, x) or
                     re.findall(ignoreincludeproblem, x) or
                     re.findall(ignoreonlinebook, x) or
                     re.findall(ignoretrackmatching, x) or
                     re.findall(ignoretoolsdoc, x) or
                     re.findall(ignoretrackingeventtimeextraction, x),
                     ['--light']
                     )
    if not light_build:
        check_error_free("b2code-sphinx-warnings", "sphinx", None,
                         lambda x:
                         re.findall(ignoretoolsdoc, x)
                         )
