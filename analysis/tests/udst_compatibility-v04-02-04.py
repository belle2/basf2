#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test backwards compatibility for a udst file produced with release-04-01-04.

This is not always guaranteed. Analysis objects may break backwards
compatibility for older files. Some niche corner cases might be problematic for
file produced with release older than release-05-00-00.

See https://confluence.desy.de/display/BI/Backward+Compatibility for more
"""

import basf2
import b2test_utils
import udst

if __name__ == "__main__":

    # clear out stray env variables and set seeding for reproducible results
    b2test_utils.configure_logging_for_tests()
    basf2.set_random_seed(1)

    # configure processing path - input file is the first 3 events from Phil's
    # favourite test udst file from https://questions.belle2.org/question/9758/
    main = basf2.create_path()
    main.add_module(
        "RootInput", inputFileName=basf2.find_file("analysis/tests/udst-v04-02-04.root")
    )
    main.add_module("EventInfoPrinter")

    # this function does the hard work
    udst.add_udst_dump(main, True)

    # also dump some variables just in case the DataStorePrinter hides some
    # problems (like has happened once: PR #7525 / BII-7148).
    sanity_check_variables = [
        "mcPDG",
        "daughter(0, mcPDG)",  # check the MCParticle <--> Particle relation (or array index lookup)
        "Mbc", "InvM", "deltaE",
        "daughter(1, pionID)",  # check the PIDLikelihoods <--> Particle relation
        "daughter(1, kaonID)",
        "daughter(1, clusterE)",  # check the ECLCluster <--> Particle relation (or array index lookup)
        "daughter(1, klmClusterLayers)",  # check the KLMCluster <--> Particle relation (or array index lookup)
    ]
    main.add_module(
        "ParticlePrinter", listName="B0:semileptonic", variables=sanity_check_variables)

    # only 3 events in the file, but set explicitly case this test script gets copied
    basf2.process(main, 3)
