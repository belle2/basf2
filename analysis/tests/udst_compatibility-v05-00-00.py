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
Test backwards compatibility for a udst file produced with release-05-00-00.

This is the *start* of analysis objects guarantee of backwards compatibility.
Pragmatically, we are backwards compatible with release-04 **in** release-05
but we cannot guarantee this in general.

See https://confluence.desy.de/display/BI/Backward+Compatibility for more
specific statements about guarantees, and in case you observe a test failure.

We keep a coarse log of the changes here for reference. Please reference a
JIRA ticket wherever possible...

CHANGES between release-04-00-00 and release-05-00-00:
    * Renamed m_particleType to m_particleSource (a more meaningful name).
      See BII-3959 and BII-7148.

CHANGES since release-05-00-00:
    * Added m_momentumScale to handle the momentum scale correction for
      TreeFitter. See BII-6971.
"""

import basf2
import b2test_utils
import udst

if __name__ == "__main__":

    # clear out stray env variables and set seeding for reproducible results
    b2test_utils.configure_logging_for_tests()
    basf2.set_random_seed(1)

    # configure processing path - input file is a single event from the feiSL
    # skim run over the file mdst/tests/mdst-v05-00-00.root
    main = basf2.create_path()
    main.add_module(
        "RootInput", inputFileName=basf2.find_file("analysis/tests/udst-v05-00-00.root")
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
    for listname in ["B+:feiSL", "B-:feiSL", "B0:feiSL"]:
        main.add_module(
            "ParticlePrinter", listName=listname, variables=sanity_check_variables)

    # only 1 event in the file, but set explicitly case this test script gets copied
    basf2.process(main, 1)
